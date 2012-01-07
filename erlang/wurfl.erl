-module(wurfl).

-behaviour(gen_server).

-export([
	parse/1,
	xhtml/2,
	fall_back/2,
	getid/2,
	xhtml_with_fall_back/2,
	get_all_ua/2,
	test/1
]).

%% gen_server
%% use gen_server
-define(SERVER, ?MODULE).
-export([
	parse_gen/1,
	xhtml_gen/1,
	fall_back_gen/1,
	getid_gen/1,
	xhtml_with_fall_back_gen/1,
	get_all_ua_gen/1,
	test_gen/1
	]).
	
%% gen_server callbacks
-export([init/1, handle_call/3, handle_cast/2, handle_info/2,
         terminate/2, code_change/3]).

parse_gen(Name) ->
	gen_server:start_link({local, ?SERVER}, ?MODULE, [Name], []).

xhtml_gen(Key) ->
	gen_server:call(?SERVER, {xhtml,{Key}}).

fall_back_gen(Key) ->
	gen_server:call(?SERVER, {fall_back,{Key}}).
	
getid_gen(Key) ->
	gen_server:call(?SERVER, {getid,{Key}}).
	
xhtml_with_fall_back_gen(Key) ->
	gen_server:call(?SERVER, {xhtml_with_fall_back,{Key}}).
	
get_all_ua_gen(Key) ->
	gen_server:call(?SERVER, {get_all_ua,{Key}}).
	
test_gen(Name) ->
	T1 = megatime(),
	io:format ("Parse started ~p~n",[T1]),
	parse_gen (Name),
	T2 = megatime(),
	io:format ("Parse stoped ~p~n",[{T2 - T1, T2}]),
	T3 = megatime(),
	io:format ("Get Id started ~p~n",[T3]),
	R1 = getid_gen ("Mozilla/4.0 (compatible; MSIE 4.01; Windows CE; i-mate JAMin PPC; 240x320; PPC; 240x320)"),
	T4 = megatime(),
	io:format ("Get Id stoped ~p~n",[{R1, T4 - T3, T4}]),
	T5 = megatime(),
	io:format ("Get xhmtl(direct) started ~p~n",[T5]),	
	R2 = xhtml_gen ("imate_jamin_ver1"),
	T6 = megatime(),
	io:format ("Get xhmtl(direct) stoped ~p~n",[{R2, T6 - T5, T6}]),
	T7 = megatime(),
	io:format ("Get xhmtl(indirect) started ~p~n",[T7]),	
	R3 = xhtml_with_fall_back_gen ("htc_prophet_ver1_dopod830"),
	T8 = megatime(),
	io:format ("Get xhmtl(indirect) stoped ~p~n",[{R3, T8 - T7, T8}]),
	T9 = megatime(),
	io:format ("Get id(indirect) started ~p~n",[T9]),	
	R4 = get_all_ua_gen ("Nokia-WAP-Toolkit/1"),
	T10 = megatime(),
	io:format ("Get id(indirect) stoped ~p~n",[{R4, T10 - T9, T10}]).
	
%%callback's	
init([Name]) ->	
	{ok, parse(Name)}.
	 
handle_call({xhtml, {Key}}, _From, State) ->
	{reply, xhtml(State,Key), State};

handle_call({fall_back, {Key}}, _From, State) ->
	{reply, fall_back(State,Key), State};

handle_call({getid, {Key}}, _From, State) ->
	{reply, getid(State,Key), State};
	
handle_call({xhtml_with_fall_back, {Key}}, _From, State) ->
	{reply, xhtml_with_fall_back(State,Key), State};

handle_call({get_all_ua, {Key}}, _From, State) ->
	{reply, get_all_ua(State,Key), State}.

handle_cast(_Msg, State) ->
	{noreply, State}.

handle_info(_Info, State) ->
	{noreply, State}.

terminate(_Reason, _State) -> 
	ok.
	
code_change(_OldVsn, State, _Extra) ->
	{ok, State}.
	
%% /gen_server

parse (Name) ->
	case xmerl_scan:file(Name) of
	{R,_} ->
		R;
	N ->
		io:format("Some other result ~p~n",[N]),
		undefined
	end.

get_value_only ([{xmlAttribute, _, _, _, _, _, _, _, Value , _}], UA) ->
	Ret = string:str (Value, UA),
	if Ret == 0 ->
		[];
	true ->
		[Value]
	end;

get_value_only ([{xmlAttribute, _, _, _, _, _, _, _, Value , _}|Tail], UA) ->
	Ret = string:str (Value, UA),
	if Ret == 0 ->
		[] ++ get_value_only (Tail, UA);
	true ->
		[Value] ++ get_value_only (Tail, UA)
	end;

get_value_only (Any, _) ->
	io:format("Some other result ~p~n",[Any]),
	undefined.

get_all_ua (Parsed, UA) ->
	XPath =  "/wurfl/devices/device/@user_agent",
	XHTML = xmerl_xpath:string (XPath, Parsed),
	get_value_only (XHTML, UA).
		
%%wurfl:getid (P,"Mozilla/4.0 (compatible; MSIE 4.01; Windows CE; i-mate JAMin PPC; 240x320; PPC; 240x320)").
getid (Parsed, UA) ->
	XPath = "/wurfl/devices/device[@user_agent='" ++
		UA ++ 
		"']/@id",
	XHTML = xmerl_xpath:string (XPath, Parsed),
	case XHTML of
	[{xmlAttribute,id, _, _, _, _, _, _, Value, _}] ->
			Value;
	N ->
		io:format("Some other result ~p~n",[{XPath,N}]),
		undefined
	end.


%%wurfl:fall_back (P,"imate_jamin_ver1").
fall_back (Parsed, ID) ->
	XPath = "/wurfl/devices/device[@id='" ++
		ID ++ 
		"']/@fall_back",
	XHTML = xmerl_xpath:string (XPath, Parsed),
	case XHTML of
	[{xmlAttribute,fall_back, _, _, _, _, _, _, Value, _}] ->
			Value;
	N ->
		io:format("Some other result ~p~n",[{XPath,N}]),
		undefined
	end.

%%wurfl:xhtml (P,"imate_jamin_ver1").
xhtml (Parsed, ID) ->
	XPath = "/wurfl/devices/device[@id='" ++
		ID ++ 
		"']/group[@id='markup']/capability[@name='html_wi_oma_xhtmlmp_1_0']/@value",
	XHTML = xmerl_xpath:string (XPath, Parsed),
	case XHTML of
	[{xmlAttribute,value, _, _, _, _, _, _, Value, _}] ->
			Value;
	N ->
		io:format("Some other result ~p~n",[{XPath,N}]),
		undefined
	end.

%%wurfl:xhtml_with_fall_back (P, "htc_prophet_ver1_dopod830").
xhtml_with_fall_back (Parsed, ID) ->
	XHTML = xhtml (Parsed, ID),
	case XHTML of
	undefined ->
		Fall_back = fall_back (Parsed, ID),
		case Fall_back of
		undefined ->
			io:format("xhtml? ~p~n",[ID]);
		NewID ->
			xhtml_with_fall_back (Parsed, NewID)
		end;
	N ->
		N
	end.

megatime() ->
	{MegaSecs,Secs,Microsecs} = now(),
	1000000 * ( 1000000 * MegaSecs + Secs ) + Microsecs.
 
test(Name)->
	T1 = megatime(),
	io:format ("Parse started ~p~n",[T1]),
	R0 = parse (Name),
	T2 = megatime(),
	io:format ("Parse stoped ~p~n",[{T2 - T1, T2}]),
	T3 = megatime(),
	io:format ("Get Id started ~p~n",[T3]),
	R1 = getid (R0,"Mozilla/4.0 (compatible; MSIE 4.01; Windows CE; i-mate JAMin PPC; 240x320; PPC; 240x320)"),
	T4 = megatime(),
	io:format ("Get Id stoped ~p~n",[{R1, T4 - T3, T4}]),
	T5 = megatime(),
	io:format ("Get xhmtl(direct) started ~p~n",[T5]),	
	R2 = xhtml (R0,"imate_jamin_ver1"),
	T6 = megatime(),
	io:format ("Get xhmtl(direct) stoped ~p~n",[{R2, T6 - T5, T6}]),
	T7 = megatime(),
	io:format ("Get xhmtl(indirect) started ~p~n",[T7]),	
	R3 = xhtml_with_fall_back (R0, "htc_prophet_ver1_dopod830"),
	T8 = megatime(),
	io:format ("Get xhmtl(indirect) stoped ~p~n",[{R3, T8 - T7, T8}]),
	T9 = megatime(),
	io:format ("Get id(indirect) started ~p~n",[T9]),	
	R4 = get_all_ua (R0,"Nokia-WAP-Toolkit/1"),
	T10 = megatime(),
	io:format ("Get id(indirect) stoped ~p~n",[{R4, T10 - T9, T10}]).
