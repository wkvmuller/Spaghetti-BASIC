awk '
/^void execute/ {
    sub(/;$/, "")               # remove the trailing semicolon
    verb = $0
    sub(/^void execute/, "", verb)
    sub(/\(.*/, "", verb)       # extract XXX
    print $0 " { std::cout << \"Stub of " verb "\" << std::endl; }"
    next
}
{ print }                       # print other lines unchanged
' proto.txt > stub_output.cpp
