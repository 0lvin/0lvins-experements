from HTMLParser import HTMLParser
import re
import cssmin
from webassets.filter.rjsmin.rjsmin import jsmin

REMOVE_WS = re.compile(r"\s{2,}").sub

# create a subclass and override the handler methods
class HTMLMinimize(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.result = ''
        self.inside_pre = False
        self.inside_style = False
        self.inside_script = False
        self.inside_textarea = False

    def handle_starttag(self, tag, attrs):
        if "pre" == tag.lower():
            self.inside_pre = True
        if "style" == tag.lower():
            self.inside_style = True
        if "script" == tag.lower():
            self.inside_script = True
        if "textarea" == tag.lower():
            self.inside_textarea = True            
        self.result += REMOVE_WS(" ", self.get_starttag_text())

    def handle_startendtag(self, tag, attributes):
        self.handle_starttag(tag, attributes)

    def handle_endtag(self, tag):
        if "pre" == tag.lower():
            self.inside_pre = False
        if "style" == tag.lower():
            self.inside_style = False
        if "script" == tag.lower():
            self.inside_script = False
        if "textarea" == tag.lower():
            self.inside_textarea = False
        self.result += ("</" + tag + ">")

    def handle_data(self, data):
        if not self.inside_pre and \
           not self.inside_style and \
           not self.inside_script :
            data = data.strip()
            data = REMOVE_WS(" ", data)
        if self.inside_style:
            data = cssmin.cssmin(data)
        if self.inside_script:
            data = jsmin(data)
        self.result += (data)

    def handle_charref(self, name):
        self.result += ("&#" + name + ";")

    def handle_entityref(self, name):
        self.result += ("&" + name + ";")

    def handle_comment(self, data):
        if '[if' in data:
            self.result += ("<!--" + data + "-->")

    def handle_decl(self, data):
        self.result += ("<!" + data + ">")

    def handle_pi(self, data):
        return
