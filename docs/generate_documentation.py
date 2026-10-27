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


# Parse the md file for function names and documentation
def parse_markdown_file():
    md_file = open("sound_lib.md").read()
    lines = md_file.split("\n")
    function_list = {}

    current_function = ""
    current_desc = ""

    for l in lines:
        l = l.strip()
        if l.startswith("##"):
            name = l.removeprefix("##")
            name = name.strip()

            if current_function:
                function_list[current_function] = current_desc

            current_function = name

            # Reset description
            current_desc = ""
        else:
            current_desc += l + "\n"


    # Add the final function
    if current_function:
        function_list[current_function] = current_desc

    return function_list

def generate_html(output_name, function_list, header_functions):
    sb = string_builder()


    sb.write("%s",
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
    }

    hr {
        border: none;
        height: 1px;
        color: #333;
        background-color: #333;
        
    }

    </style>
    """
    )

    sb.write("<body>\n")

    ignored_funcs = ("ma_sleep", "Sound_Load", "Sound_Play", "Sound_Stop")

    for func in header_functions:
        if func not in function_list and func not in ignored_funcs:
            printf("Warning: Function '%s' is missing from the documentation!!!\n", func)

    # Generate title
    title_heading = "Sound Lib Documentation"
    sb.write("<center>\n")
    sb.write("<h1>%s</h1>", title_heading)
    sb.write("</center>\n")


    # Generate links
    sb.write("<ul>\n")
    for func, _ in function_list.items():
        sb.write("<li>")
        sb.write("<a href=#%s>%s</a>\n", func, func)
        sb.write("</li>")
    sb.write("</ul>\n")

    for func, desc in function_list.items():
        sb.write("\n")
        sb.write("<h2 id=%s>%s</h2>\n", func, func)
        desc_lines = desc.split("\n")

        if func in header_functions:
            signature = header_functions[func]
            sb.write("<h3>Syntax</h3>\n")
            sb.write("<div class=\"codeblock\">")
            sb.write("<code class=\"sourcecode\">%s</code>\n", signature)
            sb.write("</div>\n")
            sb.write("<br>\n")
        else:
            printf("WARNING: Function %s is missing from the header!!!\n", func)


        for line in desc_lines:
            sb.write("<p class=\"desc\">%s</p>\n", line)

        sb.write("<hr>\n")

        # print(f)
        # print(desc)

    sb.write("</body>\n")

    sb.save(output_name)


function_list = parse_markdown_file()
header_functions = parse_header()

generate_html("sound_lib.html", function_list, header_functions)


# https://stackoverflow.com/questions/4151743/how-can-i-change-the-thickness-of-my-hr-tag

