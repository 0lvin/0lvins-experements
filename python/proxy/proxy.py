import socket, thread, select

def get_connection_info(header):
    commands = [
        'OPTIONS',
        'GET',
        'HEAD',
        'POST',
        'PUT',
        'DELETE',
        'TRACE',
        'CONNECT'
    ]
    header_lines = header.split("\r\n")
    if not len(header_lines):
        return {
            "error": "empty_header"
        }
    first_line = header_lines[0].split(" ")
    method = first_line[0]
    url = first_line[1]
    version = first_line[2]
    if method not in commands:
        return {
            "error": "unknow command"
        }
    host = ""
    port = 80
    result_header = []
    keepalive = ''
    size = None
    for header_line in header_lines[1:]:
        if header_line.lower().find("host:") == 0:
            host = header_line[len("host:"):]
        elif header_line.lower().find("proxy-connection:") == 0:
            keepalive = header_line[len("proxy-connection:"):]
            result_header += ["Connection:" + keepalive]
        elif header_line.lower().find("content-length:") == 0:
            result_header += [header_line]
            size = int(header_line[len("content-length:"):].strip())
        else:
            if len(header_line.strip()):
                result_header += [header_line]
    for protocol_type in ["http", "https"]:
        prefixes = protocol_type + "://"
        prefix = url.lower().find(prefixes)
        if prefix == 0:
            host_url = url[len(prefixes):]
            host = host_url[:host_url.find('/')]
            url = host_url[host_url.find('/'):]
    if method.lower() == "connect":
        host = url
        url = ""
    if host.find(":") != -1:
        port = host[host.find(":") + 1:]
        host = host[:host.find(":")]
    return {
        "method": method,
        "version" : version,
        "host": host,
        "port": port,
        "url": url,
        "headers": result_header,
        "size": size,
        "keep-alive": keepalive.strip() == "keep-alive"
    }

def download(conn, size=None, stop=None):
    buffer = ""
    while 1:
        data = None
        try:
            data = conn.recv(1024)
            print "download=", len(data)
        except socket.error as e:
            print e
        if not data:
            return buffer
        buffer += data
        if size is not None and len(buffer) >= size:
            return buffer
        if stop is not None and buffer.find(stop) != -1:
            return buffer

connectio_pool = {}

def get_connection(host, port):
    print "to:" + host + ":" + str(port) + "\n"
    s = connectio_pool.get(host + ":" + str(port))
    if not s:
        for res in socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            try:
                s = socket.socket(af, socktype, proto)
            except socket.error as msg:
                s = None
                continue
            try:
                s.connect(sa)
            except socket.error as msg:
                s.close()
                s = None
                continue
            break
    return s

def get_response(s, conn, close):
    server_buffer = download(s, stop='\r\n\r\n')
    header_found = server_buffer.find('\r\n\r\n')
    if header_found !=-1:
        headers = server_buffer[:header_found + 4]
        chunked = False
        size = None
        result_headers = []
        for header in headers.split("\r\n"):
            if header.lower().find("transfer-encoding") == 0:
                value = header[len("transfer-encoding")+1:]
                value = value.strip()
                if value.lower() == "chunked":
                    chunked = True
                result_headers += [header]
            elif header.lower().find("content-length") == 0:
                value = header[len("content-length")+1:]
                value = value.strip()
                size = int(value)
                result_headers += [header]
            elif header.lower().find("connection") == 0:
                value = header[len("connection")+1:]
                value = value.strip()
                if value.lower() == "close":
                    close = True
            else:
                if len(header.strip()):
                    result_headers += [header]
        if not size and close:
            result_headers += ["Transfer-Encoding: chunked"]

        print "<---\r\n" + "\r\n".join(result_headers)
        server_buffer = server_buffer[header_found + 4:]
        conn.sendall("\r\n".join(result_headers) + '\r\n\r\n')
        if chunked:
            size = 1
            while size != 0:
                if server_buffer.find("\r\n") != -1:
                    hex_size = server_buffer[:server_buffer.find("\r\n")]
                    size = int(hex_size, 16)
                    server_buffer = server_buffer[server_buffer.find("\r\n")+2:]
                    if size > len(server_buffer):
                        server_buffer += download(s, size=(size - len(server_buffer) + len("0\r\n")))
                    conn.sendall(hex_size + "\r\n" + server_buffer[:size])
                    server_buffer = server_buffer[size:]
                    if server_buffer[:2] == "\r\n":
                        conn.sendall("\r\n")
                        server_buffer = server_buffer[2:]
                    else:
                        print "strange"
                        exit(1)
                    print "hex:", hex_size
                else:
                    server_buffer += download(s, size=10)
            #conn.sendall("0" + "\r\n")
        else:
            # exist content length
            if size:
                if size > len(server_buffer):
                    server_buffer += download(s, size=size-len(server_buffer))
                print "send", len(server_buffer)
                conn.sendall(server_buffer)
            else:
                #chunck emulate
                server_buffer += download(s)
                conn.sendall(hex(len(server_buffer)) + "\r\n" + server_buffer)
                print hex(len(server_buffer)) + "\r\n" + server_buffer
                #close stream
                conn.sendall("\r\n0\r\n")
    print close
    return not close

def connection(conn):
    client_buffer = ""
    while 1:
        close = True
        data = download(conn, stop='\r\n\r\n')
        client_buffer += data
        header_found = client_buffer.find('\r\n\r\n')
        if header_found!=-1:
            header = client_buffer[:header_found]
            client_buffer = client_buffer[header_found + len('\r\n\r\n'):]
            connection_info = get_connection_info(header)
            if 'error' in connection_info:
                print header
                exit(1)
            s = get_connection(connection_info['host'], connection_info['port'])
            if s is None:
                print 'could not open socket'
                sys.exit(1)
            if connection_info['method'].lower() == "connect":
                close = True
                conn.sendall(connection_info['version'] + ' 200 Connection established\r\n'+
                         'Proxy-agent: transparent\r\n\r\n')
                sockets = [s, conn]
                s.sendall(client_buffer)
                client_buffer = ""
                while 1:
                    (sockets_changes, _, err) = select.select(sockets, [], sockets, 10)
                    if err:
                        break
                    if sockets_changes:
                        for in_socket in sockets_changes:
                            client_buffer = in_socket.recv(1024)
                            if in_socket is conn:
                                out_socket = s
                            else:
                                out_socket = conn
                            if client_buffer:
                                out_socket.send(client_buffer)
            else:
                for_send = [connection_info['method'] + " " + connection_info['url'] +  " " + connection_info['version']]
                for_send += ["Host:" + connection_info['host']]
                for_send += connection_info['headers']
                print "--->\r\n" + "\r\n".join(for_send)
                s.sendall("\r\n".join(for_send) + "\r\n\r\n")
                if connection_info['method'].lower() == "post":
                    if connection_info['size']:
                        size = connection_info['size']
                        if len(client_buffer) < size:
                            client_buffer += download(conn, size=size-len(client_buffer))
                        s.sendall(client_buffer[:size])
                        client_buffer = client_buffer[size:]
                    else:
                        print "post without size"
                        client_buffer += download(conn)
                        s.sendall(client_buffer)
                        client_buffer = ""
                if not get_response(s, conn, not connection_info["keep-alive"]):
                    s.close()
                    connectio_pool[connection_info['host'] + ":" + str(connection_info['port'])] = None
                else:
                    connectio_pool[connection_info['host'] + ":" + str(connection_info['port'])] = s
                if not connection_info["keep-alive"]:
                    print "not keep alive"
                    break
        if not len(data):
            print "no data from browser"
            break
        print "next round"

# Symbolic name meaning all available interfaces
HOST = '127.0.0.1'
# Arbitrary non-privileged port
PORT = 8080
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()
connection(conn)
exit(0)
while 1:
    conn, addr = s.accept()
    print "connected", addr
    thread.start_new_thread(connection, (conn,))
