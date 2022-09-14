# *******************************************************************************************
# *******************************************************************************************
#
#		Name : 		ps2codes.py
#		Purpose :	Generate PS/2 tables etc.
#		Date :		4th August 2022
#		Author : 	Paul Robson (paul@robsons.org.uk)
#
# *******************************************************************************************
# *******************************************************************************************

import re 

# *******************************************************************************************
#
#		What we want is a table that maps PS/2 keycodes 00-7F onto SDL Key Codes (SDLK_)
#
#		The program then inverses this and uses it to generate a SDL Key code => PS/2 Keycode
#		table
#
# *******************************************************************************************

# *******************************************************************************************
#
#							This is scan code, unshifted, shifted data
#
# *******************************************************************************************

rawPS2data = """
16/F0 16||1||!
1E/F0 1E||2||@
26/F0 26||3||#
25/F0 25||4||$
2E/F0 2E||5||%
36/F0 36||6||^
3D/F0 3D||7||&
3E/F0 3E||8||*
46/F0 46||9||(
45/F0 45||0||)
4E/F0 4E||-||_
55/F0 55||=||+
66/F0 66||Backspace||
0D/F0 0D||Tab||
15/F0 15||q||Q
1D/F0 1D||w||W
24/F0 24||e||E
2D/F0 2D||r||R
2C/F0 2C||t||T
35/F0 35||y||Y
3C/F0 3C||u||U
43/F0 43||i||I
44/F0 44||o||O
4D/F0 4D||p||P
54/F0 54||[||{
5B/F0 5B||]||}
1C/F0 1C||a||A
1B/F0 1B||s||S
23/F0 23||d||D
2B/F0 2B||f||F
34/F0 34||g||G
33/F0 33||h||H
3B/F0 3B||j||J
42/F0 42||k||K
4B/F0 4B||l||L
4C/F0 4C||;||:
52/F0 52||'||"
5A/F0 5A||Enter||Enter
12/F0 12||Left Shift||
1A/F0 1A||z||Z
22/F0 22||x||X
21/F0 21||c||C
2A/F0 2A||v||V
32/F0 32||b||B
31/F0 31||n||N
3A/F0 3A||m||M
41/F0 41||,||<
49/F0 49||.||>
4A/F0 4A||/||?
59/F0 59||Right Shift||
14/F0 14||Left Ctrl||
11/F0 11||Left Alt||
29/F0 29||Space||
76/F0 76||Esc||
5D/F0 5D||\||
"""

# *******************************************************************************************
#
#							This is Text, Scan Code, SDLK Code
#
# *******************************************************************************************

rawSDLData = """
"0"|SDL_SCANCODE_0|SDLK_0
"1"|SDL_SCANCODE_1|SDLK_1
"2"|SDL_SCANCODE_2|SDLK_2
"3"|SDL_SCANCODE_3|SDLK_3
"4"|SDL_SCANCODE_4|SDLK_4
"5"|SDL_SCANCODE_5|SDLK_5
"6"|SDL_SCANCODE_6|SDLK_6
"7"|SDL_SCANCODE_7|SDLK_7
"8"|SDL_SCANCODE_8|SDLK_8
"9"|SDL_SCANCODE_9|SDLK_9
"A"|SDL_SCANCODE_A|SDLK_a
"'"|SDL_SCANCODE_APOSTROPHE|SDLK_QUOTE
"B"|SDL_SCANCODE_B|SDLK_b
"\\" |SDL_SCANCODE_BACKSLASH|SDLK_BACKSLASH
"Backspace"|SDL_SCANCODE_BACKSPACE|SDLK_BACKSPACE
"C"|SDL_SCANCODE_C|SDLK_c
"CapsLock"|SDL_SCANCODE_CAPSLOCK|SDLK_CAPSLOCK
","|SDL_SCANCODE_COMMA|SDLK_COMMA
"D"|SDL_SCANCODE_D|SDLK_d
"Delete"|SDL_SCANCODE_DELETE|SDLK_DELETE
"Down"|SDL_SCANCODE_DOWN|SDLK_DOWN
"E"|SDL_SCANCODE_E|SDLK_e
"End"|SDL_SCANCODE_END|SDLK_END
"="|SDL_SCANCODE_EQUALS|SDLK_EQUALS
"Esc" |SDL_SCANCODE_ESCAPE|SDLK_ESCAPE
"F"|SDL_SCANCODE_F|SDLK_f
"F1"|SDL_SCANCODE_F1|SDLK_F1
"F10"|SDL_SCANCODE_F10|SDLK_F10
"F11"|SDL_SCANCODE_F11|SDLK_F11
"F12"|SDL_SCANCODE_F12|SDLK_F12
"F2"|SDL_SCANCODE_F2|SDLK_F2
"F3"|SDL_SCANCODE_F3|SDLK_F3
"F4"|SDL_SCANCODE_F4|SDLK_F4
"F5"|SDL_SCANCODE_F5|SDLK_F5
"F6"|SDL_SCANCODE_F6|SDLK_F6
"F7"|SDL_SCANCODE_F7|SDLK_F7
"F8"|SDL_SCANCODE_F8|SDLK_F8
"F9"|SDL_SCANCODE_F9|SDLK_F9
"G"|SDL_SCANCODE_G|SDLK_g
"`" |SDL_SCANCODE_GRAVE|SDLK_BACKQUOTE
"H"|SDL_SCANCODE_H|SDLK_h
"Home"|SDL_SCANCODE_HOME|SDLK_HOME
"I"|SDL_SCANCODE_I|SDLK_i
"Insert"|SDL_SCANCODE_INSERT|SDLK_INSERT
"J"|SDL_SCANCODE_J|SDLK_j
"K"|SDL_SCANCODE_K|SDLK_k
"L"|SDL_SCANCODE_L|SDLK_l
"Left Alt"|SDL_SCANCODE_LALT|SDLK_LALT
"Left Ctrl"|SDL_SCANCODE_LCTRL|SDLK_LCTRL
"Left"|SDL_SCANCODE_LEFT|SDLK_LEFT
"["|SDL_SCANCODE_LEFTBRACKET|SDLK_LEFTBRACKET
"Left Shift"|SDL_SCANCODE_LSHIFT|SDLK_LSHIFT
"M"|SDL_SCANCODE_M|SDLK_m
"-"|SDL_SCANCODE_MINUS|SDLK_MINUS
"N"|SDL_SCANCODE_N|SDLK_n
"O"|SDL_SCANCODE_O|SDLK_o
"P"|SDL_SCANCODE_P|SDLK_p
"PageDown"|SDL_SCANCODE_PAGEDOWN|SDLK_PAGEDOWN
"PageUp"|SDL_SCANCODE_PAGEUP|SDLK_PAGEUP
"."|SDL_SCANCODE_PERIOD|SDLK_PERIOD
"Q"|SDL_SCANCODE_Q|SDLK_q
"R"|SDL_SCANCODE_R|SDLK_r
"Right Alt" |SDL_SCANCODE_RALT|SDLK_RALT
"Right Ctrl"|SDL_SCANCODE_RCTRL|SDLK_RCTRL
"Enter"|SDL_SCANCODE_RETURN|SDLK_RETURN
"Right"|SDL_SCANCODE_RIGHT|SDLK_RIGHT
"]"|SDL_SCANCODE_RIGHTBRACKET|SDLK_RIGHTBRACKET
"Right Shift"|SDL_SCANCODE_RSHIFT|SDLK_RSHIFT
"S"|SDL_SCANCODE_S|SDLK_s
"ScrollLock"|SDL_SCANCODE_SCROLLLOCK|SDLK_SCROLLLOCK
";"|SDL_SCANCODE_SEMICOLON|SDLK_SEMICOLON
"/"|SDL_SCANCODE_SLASH|SDLK_SLASH
"Space"|SDL_SCANCODE_SPACE|SDLK_SPACE
"Stop"|SDL_SCANCODE_STOP|SDLK_STOP
"SysReq" |SDL_SCANCODE_SYSREQ|SDLK_SYSREQ
"T"|SDL_SCANCODE_T|SDLK_t
"Tab" |SDL_SCANCODE_TAB|SDLK_TAB
"U"|SDL_SCANCODE_U|SDLK_u
"Up"|SDL_SCANCODE_UP|SDLK_UP
"V"|SDL_SCANCODE_V|SDLK_v
"W"|SDL_SCANCODE_W|SDLK_w
"X"|SDL_SCANCODE_X|SDLK_x
"Y"|SDL_SCANCODE_Y|SDLK_y
"Z"|SDL_SCANCODE_Z|SDLK_z
"""

#
#		Create a text to SDLK_Mapping
#
keyToSDKKeycode = {}

for s in rawSDLData.replace(" ","").strip().split("\n"):
	m = re.match("^\"(.*?)\"\\|.*?\\|(.*)$",s)
	key = m.group(1).strip().upper()
	assert key not in keyToSDKKeycode
	keyToSDKKeycode[key] = m.group(2).strip()
#print(keyToSDKKeycode.keys())
#
#		Now create a table for each scancode which has its corresponding SDLK_ constant.
#
keyCodeTable = [ None ] * 144

for s in rawPS2data.upper().replace(" ","").strip().split("\n"):
	m = re.match("^([0-9A-F][0-9A-F]).*?\\|\\|(.*?)\\|\\|(.*?)\\s*$",s)
	name = m.group(2)
	n = int(m.group(1),16)
	assert name in keyToSDKKeycode,"Unknown "+name
	assert n > 0 and n < 0x90,"Bad "+s
	assert keyCodeTable[n] is None
	keyCodeTable[n] = keyToSDKKeycode[name]

table = ",".join([x if x is not None else "0" for x in keyCodeTable])

print("//\n//\t This file is automatically generated.\n//")
print("static int sdlKeySymbolList[] = {{ {0},-1 }};\n\n".format(table))