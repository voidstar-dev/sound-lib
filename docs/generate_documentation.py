# Documentation generator for sound_lib.h
# Note that this is not a real C parser.
# It exploits some common patterns found in the header file for easier parsing.
# This file uses sound_lib.md as an auxillary file for the actual documentation
# Function name matching is performed during document compilation, to ensure that function names match between
# the .md file and the .h file.

# The first line (not sentence!) of each function section serves as a brief
# summary. The subsequent lines go into the "Remarks" section, which should
# contain more detailed information about the function.

# The "See Also" section should contain one or more lines, with each line containing the name of a single function.


# =============================================================================

# .md syntax

# ## <Function Name>
# <Brief>
# <Desc>


# ### Returns
# <Optional return desc>


# ### See Also
# <Optional see also desc>


# See sound_lib.md for examples.


# The 'Returns' and 'See Also' headings are optional.
# Currently, we do a simple check on the function signature. If the return type is not void,
# we warn that the 'Returns' heading is missing.

# =============================================================================



from pathlib import Path
import time
import re

def sprintf(fmt, *args):
    s = fmt % args
    return s

def printf(fmt, *args):
    s = fmt % args
    print(s, end='')

def write_entire_file(name, data):
    f = open(name, "w")
    f.write(data)
    f.close()

def warn(fmt, *args):
    ansi_codes = True
    heading = "WARNING: "
    s = sprintf(fmt, *args)
    if(ansi_codes): printf("\033[1;35m%s\033[0m", heading)
    else:           printf("%s", heading)

    printf("%s", s)

class string_builder:
    def __init__(self):
        self.buf = ""

    def write(self, text, *args):
        self.buf += sprintf(text, *args)

    def data(self):
        return self.buf

    def save(self, fname):
        write_entire_file(fname, self.buf)


# Parse a fuction signature in a header file.
def parse_function_signature(line: str):
    found = False
    name = ""
    signature = ""
    if line.find("SOUNDAPI") == -1: return name, signature, found
    if line.find(";") == -1: return name, signature, found
    tokens = line.split()
    if len(tokens) >= 3:
        before = tokens[2]
        name = before.split("(")[0]
        name = name.strip("*")
        signature = line.lstrip("SOUNDAPI").strip()
        found = True

    return name, signature, found

class Header():
    def __init__(self, functions, structs):
        self.functions = functions
        self.structs = structs

def parse_header():
    # Parse the header for function names and the signature
    header = open("../src/sound_lib.h").read()
    header_lines = header.split("\n")
    header_functions = {}
    structs = {}

    in_struct = False
    struct_name = ""
    struct_text = ""
    regex = None

    for line in header_lines:
        # Blank lines are significant in struct readability
        if not in_struct and len(line) == 0: continue

        # Try to parse struct
        if line.startswith("typedef struct"):
            assert(not in_struct)
            tokens = line.split()
            if len(tokens) == 3:
                if not line.endswith(";"):
                    in_struct = True
                    struct_name = line.replace("typedef struct", "").strip()
                    struct_text = line + "\n"
                    expr = sprintf("} %s;", struct_name)
                    regex = re.compile(expr)
                    continue



        if in_struct:
            struct_text += line + "\n"
            matches = re.match(regex, line)
            if matches:
                in_struct = False
                assert(struct_name not in structs)
                structs[struct_name] = struct_text
        else:
            # Try to parse a function
            func_name, func_signature, found = parse_function_signature(line)
            if found:
                header_functions[func_name] = func_signature



    header = Header(header_functions, structs)

    return header


class function_attributes():
    def __init__(self, brief, remarks, return_desc, see_also):
        self.brief = brief
        self.remarks = remarks
        self.return_desc = return_desc
        self.see_also = see_also

# Parse the md file for function names and documentation
def parse_markdown_file():
    md_file = open("sound_lib.md").read()
    lines = md_file.split("\n")
    function_list = {}

    DESC_BIT        = (1 << 0)
    RETURN_BIT      = (1 << 1)
    SEE_ALSO_BIT    = (1 << 2)
    flags = 0


    current_function = ""
    current_desc = ""
    current_return = ""
    current_brief = ""
    current_see_also = ""
    in_desc = False
    in_return = False
    in_see_also = False
    parsed_lines = 0

    for l in lines:
        l = l.strip()
        parsed_lines += 1

        if l.startswith("### Returns"):
            flags = 0
            flags |= RETURN_BIT
            continue

        if l.startswith("### See Also"):
            flags = 0
            flags |= SEE_ALSO_BIT
            continue

        if l.startswith("## "):
            name = l.removeprefix("##")
            name = name.strip()

            if current_function:
                function_list[current_function] = function_attributes(current_brief, current_desc, current_return, current_see_also)

            current_function = name

            # Reset description
            current_desc = ""
            current_return = ""
            current_brief = ""
            current_see_also = ""
            flags = 0
            flags |= DESC_BIT
            parsed_lines = 0
        else:
            if flags & DESC_BIT:
                if parsed_lines == 1:
                    current_brief = l
                else:
                    current_desc += l + "\n"

            if flags & RETURN_BIT: current_return += l + "\n"
            if flags & SEE_ALSO_BIT: current_see_also += l + "\n"


    # Add the final function
    if current_function:
        function_list[current_function] = function_attributes(current_brief, current_desc, current_return, current_see_also)

    return function_list


# Given a word, and a list of keywords, find the best match
# We don't do an equality comparison because a word may not be strictly equal to
# that keyword (for example, there may be a function call as as func(), and
# we're only matching 'func')
def find_best_match(token, keywords):
    best = ""
    best_len = 0
    for k in keywords:
        if token.find(k) != -1:
            size = len(k)
            if size > best_len:
                best_len = size
                best = k
    return best


def replace_link_in_list(desc, items):
    new_desc = string_builder()
    words = desc.split()
    for w in words:
        match = find_best_match(w, items)
        if match:
            link = sprintf("<a href=#%s>%s</a>", match, match)
            w = w.replace(match, link)

        new_desc.write("%s ", w)

    return new_desc.data().strip()

def replace_function_links(desc, function_list, header):
    a = desc
    b = desc

    # We check each list of identifiers
    identifiers = [function_list, header.structs]

    for item in identifiers:
        result = replace_link_in_list(desc, item)
        if result != desc: return result

    return desc


def format_markdown(text):
    # Match backticks
    matches = re.findall(r"`[\w]+`?", text)
    matches = set(matches)
    for m in matches:
        token = m.replace("`", "")
        span = sprintf("<span class=\"inline-code\">%s</span>", token)
        text = text.replace(m, span)

    return text


# Replace function names appearing in descriptions with links
# We find the longest match for the given function
def generate_remarks(sb, desc, function_list, header):
    desc = replace_function_links(desc, function_list, header)
    desc = format_markdown(desc)
    sb.write("<h3>Remarks</h3>\n")
    if not desc:
        desc = "None."
    sb.write("<p class=\"desc\">%s</p>\n", desc)

def generate_see_also(sb, see_also, function_list, header):
    lines = see_also.split("\n")
    tmp = string_builder()
    for l in lines:
        if not l: continue

        if l in function_list or l in header.structs:
            l = replace_function_links(l, function_list, header)
            tmp.write("%s\n", l)
        else:
            warn("'See Also' line '%s' is not a function or struct!\n", l)

    data = tmp.data()

    # Only generate if we have something
    if data:
        sb.write("<h3>See Also</h3>\n")
        sb.write("<ul>\n")
        lines = data.split("\n")
        lines.pop()
        for line in lines:
            sb.write("<li>%s</li>\n", line)

        sb.write("</ul>\n")


def generate_links(sb, function_list):
    sb.write("<ul>\n")
    for func, _ in function_list.items():
        sb.write("<li>")
        sb.write("<a href=#%s>%s</a>\n", func, func)
        sb.write("</li>")
    sb.write("</ul>\n")


def generate_structs(sb, structs):
    for name, desc in structs.items():
        sb.write("<h2 id=%s>%s</h2>\n", name, name)

        sb.write("<div class=\"structblock\">\n")
        sb.write("<code class=\"structcode\">%s</code>\n", desc)
        sb.write("</div>\n")
        sb.write("<br>\n")
        sb.write("<hr>\n")

def generate_html(output_name, function_list, header):
    sb = string_builder()

    style = string_builder()

    style.write("%s",
    """
    <style>
    .desc {
      #margin: 0px;
      #margin-left: 16px;
    }


    .structcode {
        font-size: 16px;
        width: 80%;
        white-space: pre;
    }

    /* structblock is essentially the same as codeblock, except we allow horizontal scrolling */
    .structblock {
        width: 80%;
        background-color: #DDDDDD;
        padding: 8px;

        /* Scrolling */
        overflow-x: auto;
    }

    .sourcecode {
        font-size: 16px;
        width: 80%;
    }

    /* Old school 3D button-style border */
    .border-3d {
        border-width: 2px;
        border-top-color: #999999;
        border-left-color: #999999;
        border-bottom-color: #EEEEEE;
        border-right-color: #EEEEEE;
        border-style: solid;
    }

    .codeblock {
        width: 80%;
        background-color: #DDDDDD;
        padding: 8px;
    }
    

    /* Not sure if I should be using px, or em for the font-size */
    .inline-code {
        padding: 0px;
        font-weight: bold;
    }

    body {
        width: 80vw;
        margin: auto;
        overflow-wrap: break-word;
        font-family: sans;
    }

    hr {
        border: none;
        height: 1px;
        color: #333;
        background-color: #333;
    }

    /* Enable word wrap */
    p {
        max-width: 85ch;
    }

    a:active {
        color: purple;
    }

    </style>
    """
    )

    # Header variables
    header_functions = header.functions
    header_structs = header.structs


    title_heading = "Sound Lib Documentation"

    # Begin the html document
    sb.write("<!DOCTYPE html>\n")
    sb.write("<html>\n")

    # Head section
    sb.write("<head>\n")
    sb.write("<meta charset=\"utf-8\"/>\n")
    sb.write("%s\n", style.data())
    sb.write("<title>%s</title>\n", title_heading)
    sb.write("</head>\n")


    # Body section
    sb.write("<body>\n")

    ignored_funcs = ("ma_sleep", "Sound_Load", "Sound_Play", "Sound_Stop")

    for func in header_functions:
        if func not in function_list and func not in ignored_funcs:
            warn("Function '%s' is missing from the documentation\n", func)

    # Generate title
    sb.write("<center>\n")
    sb.write("<h1>%s</h1>\n", title_heading)
    sb.write("</center>\n")

    # Sort the function list based on the header order, rather than the documentation order.
    tmp = {}
    for func in header_functions:
        if func in function_list:
            tmp[func] = function_list[func]

    function_list = tmp


    generate_links(sb, function_list)
    sb.write("<hr>\n")


    # Generate name, syntax, and description
    for func, attributes in function_list.items():
        sb.write("\n")
        sb.write("<h2 id=%s>%s</h2>\n", func, func)
        return_desc = attributes.return_desc
        brief = attributes.brief
        remarks = attributes.remarks
        see_also = attributes.see_also

        # Brief one-line summary
        brief = replace_function_links(brief, function_list, header)
        sb.write("<p>%s</p>\n", brief);


        if func in header_functions:
            signature = header_functions[func]
            sb.write("<h3>Syntax</h3>\n")
            sb.write("<div class=\"codeblock\">\n")
            sb.write("<code class=\"sourcecode\">%s</code>\n", signature)
            sb.write("</div>\n")
            sb.write("<br>\n")
        else:
            warn("Function %s is missing from the header\n", func)
            continue


        # Return type
        # Todo: Pre-parse the return type
        signature = header_functions[func]
        return_type = signature.split()[0].strip().lower()
        # Return statement
        if return_desc:
            if return_type != "void":
                sb.write("<h3>Return Value</h3>\n")
                # Lowercase the first word since we always prefix the sentence with "Returns"
                return_desc = replace_function_links(return_desc, function_list, header)
                letter = return_desc[0].lower()
                sb.write("<p>Returns %c%s</p>\n", letter, return_desc[1:])
            else:
                warn("Function %s returns void, but has a 'return' section\n", func)
        else:
            if return_type != "void":
                warn("Function %s is missing a 'return' section\n", func)

        generate_remarks(sb, remarks, function_list, header)

        generate_see_also(sb, see_also, function_list, header)


        


        sb.write("<hr>\n")

    # Structs
    # Not sure where these should go. Maybe a different page?
    # The internal details of these structs are not important for most general use.
    # At the moment, only Sound_Spec is of any relevance.
    sb.write("<center>")
    sb.write("<h1>Structs</h1>")
    sb.write("</center>")
    text = "I'm not sure where to put these yet. The only one of real relevance for API users at the moment is Sound_Spec."
    text = replace_function_links(text, function_list, header)
    sb.write("<p>%s</p>", text)
    generate_structs(sb, header.structs)

    # Footer
    sb.write("<footer>\n")
    current_file_name = Path(__file__).name
    # time.strftime automatically uses the local time if we don't pass anything in
    # date_string = time.strftime("%Y-%m-%d - %H:%M:%S")
    date_string = time.strftime("%Y-%m-%d")
    sb.write("<center>\n")
    sb.write("<p>\n")
    sb.write("Generated by <i>%s</i> on <i>%s</i>\n", current_file_name, date_string)
    sb.write("</p>\n")
    sb.write("</center>\n")
    sb.write("</footer>\n")


    # Note: The footer goes within the body tag!
    sb.write("</body>\n")

    sb.write("</html>\n")

    sb.save(output_name)


function_list = parse_markdown_file()
header = parse_header()

generate_html("sound_lib.html", function_list, header)


# https://stackoverflow.com/questions/4151743/how-can-i-change-the-thickness-of-my-hr-tag

