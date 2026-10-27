# Documentation generator for sound_lib.h
# Note that this is not a real C parser.
# It exploits some common patterns found in the header file for easier parsing.
# This file uses sound_lib.md as an auxillary file for the actual documentation
# Function name matching is performed during document compilation, to ensure that function names match between
# the .md file and the .h file.


# .md syntax
# ## <Function Name>
# <Desc>

# ### Returns
# <Optional return desc>

# See sound_lib.md for examples.


# The return heading is optional.
# Currently, we do a simple check on the function signature. If the return type is not void,
# we warn if the return heading is missing.



from pathlib import Path
import time

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

def parse_header():
    # Parse the header for function names and the signature
    header = open("../sound_lib.h").read()
    header_lines = header.split("\n")
    header_functions = {}

    for line in header_lines:
        if len(line) == 0: continue
        func_name, func_signature, found = parse_function_signature(line)
        if found:
            header_functions[func_name] = func_signature

    return header_functions


class function_attributes():
    def __init__(self, desc, return_desc):
        self.desc = desc
        self.return_desc = return_desc

# Parse the md file for function names and documentation
def parse_markdown_file():
    md_file = open("sound_lib.md").read()
    lines = md_file.split("\n")
    function_list = {}

    current_function = ""
    current_desc = ""
    current_return = ""
    in_desc = False
    in_return = False

    for l in lines:
        l = l.strip()

        if l.startswith("### Returns"):
            in_return = True
            in_desc = False
            continue

        if l.startswith("## "):
            name = l.removeprefix("##")
            name = name.strip()

            if current_function:
                function_list[current_function] = function_list[current_function] = function_attributes(current_desc, current_return)

            current_function = name

            # Reset description
            current_desc = ""
            current_return = ""
            in_desc = True
            in_return = False
        else:
            if in_desc: current_desc += l + "\n"
            if in_return: current_return += l + "\n"



    # Add the final function
    if current_function:
        function_list[current_function] = function_attributes(current_desc, current_return)

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

def replace_function_links(desc, function_list):
    new_desc = string_builder()
    words = desc.split()
    for w in words:
        match = find_best_match(w, function_list)
        if match:
            link = sprintf("<a href=#%s>%s</a>", match, match)
            w = w.replace(match, link)

        new_desc.write("%s ", w)

    return new_desc.data().strip()

def generate_html(output_name, function_list, header_functions):
    sb = string_builder()

    style = string_builder()

    style.write("%s",
    """
    <style>
    .desc {
      margin: 0px;
      #margin-left: 16px;
    }

    .sourcecode {
        font-size: 16px;
        width: 80%;
    }

    .codeblock {
        width: 80%;
        background-color: #CCCCCC;
        padding: 8px;

        /* Scrolling */
        #white-space: nowrap;
        #overflow-x: auto;
    }

    body {
        width: 80vw;
        margin: auto;
        overflow-wrap: break-word;
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

    </style>
    """
    )

    # Begin the html document
    sb.write("<!DOCTYPE html>\n")
    sb.write("<html>\n")

    # Head section
    sb.write("<head>\n")
    sb.write("<meta charset=\"utf-8\"/>\n")
    sb.write("%s\n", style.data())
    sb.write("</head>\n")


    # Body section
    sb.write("<body>\n")

    ignored_funcs = ("ma_sleep", "Sound_Load", "Sound_Play", "Sound_Stop")

    for func in header_functions:
        if func not in function_list and func not in ignored_funcs:
            warn("Function '%s' is missing from the documentation\n", func)

    # Generate title
    title_heading = "Sound Lib Documentation"
    sb.write("<center>\n")
    sb.write("<h1>%s</h1>\n", title_heading)
    sb.write("</center>\n")


    # Generate links
    sb.write("<ul>\n")
    for func, _ in function_list.items():
        sb.write("<li>")
        sb.write("<a href=#%s>%s</a>\n", func, func)
        sb.write("</li>")
    sb.write("</ul>\n")

    # Generate name, syntax, and description
    for func, attributes in function_list.items():
        sb.write("\n")
        sb.write("<h2 id=%s>%s</h2>\n", func, func)
        desc = attributes.desc
        return_desc = attributes.return_desc
        desc_lines = desc.split("\n")

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


        # Replace function names appearing in descriptions with links
        # We find the longest match for the given function
        new_desc = replace_function_links(desc, function_list)
        sb.write("<p class=\"desc\">%s</p>\n", new_desc)

        signature = header_functions[func]
        return_type = signature.split()[0].strip()
        # Return statement
        if not return_desc:
            if return_type != "void":
                warn("Function %s is missing a 'return' section\n", func)
        else:
            if return_type != "void":
                sb.write("<h3>Return Value</h3>\n")
                # Lowercase the first word since we always prefix the sentence with "Returns"
                letter = return_desc[0].lower()
                sb.write("<p>Returns %c%s</p>\n", letter, return_desc[1:])
            else:
                warn("Function %s returns void, but has a 'return' section\n", func)

        sb.write("<hr>\n")

        # print(f)
        # print(desc)

    sb.write("</body>\n")

    # Footer
    sb.write("<footer>\n")
    sb.write("<p>\n")
    current_file_name = Path(__file__).name
    # time.strftime automatically uses the local time if we don't pass anything in
    date_string = time.strftime("%Y-%m-%d - %H:%M:%S")
    sb.write("<center>\n")
    sb.write("Generated by <i>%s</i> on <i>%s</i>\n", current_file_name, date_string)
    sb.write("</center>\n")
    sb.write("</p>\n")
    sb.write("</footer>\n")

    sb.write("</html>\n")

    sb.save(output_name)


function_list = parse_markdown_file()
header_functions = parse_header()

generate_html("sound_lib.html", function_list, header_functions)


# https://stackoverflow.com/questions/4151743/how-can-i-change-the-thickness-of-my-hr-tag

