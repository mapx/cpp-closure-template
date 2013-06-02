# cpp-closure-template

Google Closure Templates' c++ adapter.

It calls the official java interpreter from a jar ball through java jni interface.

## Installation
1. Download soy closure-templates source files:
<pre><code># Non-members may check out a read-only working copy anonymously over HTTP.
svn checkout http://closure-templates.googlecode.com/svn/trunk/ closure-templates-read-only</code></pre>
1. Modify build.xml to pack the template files, global parameters file and SoyHandler.java with closure-templates' stuff into the final jar ball.
1. Modify three macro in <code>src/cpp/soy_handler.h</code>:
<pre><code>MAPX_SOY_JAR_FILE
MAPX_RESOURCE_NAMESPACE
MAPX_SOYHANDLER_IN_JAR</code></pre>

1. Use <code>SoyHandler::render()</code> to render a template of tpl_name with data json_map and locale. Then use <code>SoyHandler::ngx_str()</code> to fetch the result string in a special object (it's nginx's inner string type). The string is available until the next render call. No need to release the result string's memory.

## FAQ

**Q: Why return an nginx string object rather than a null ended string?**

There is no end_of_string character like C in Java. By returning an nginx string we make zero copy of the result string.


**Q: Why do you have these large functions kept in .h file, not in .cpp file?  It's not good to inline large function.**

Just for convenience. A modern c++ compile can tell whether to inline a function and make a  good choice.

**Q: Will you write a native c++ templates interpreter?**

It depends. I wish to write it, too.

**Q: The code is ugly.**

Indeed.
