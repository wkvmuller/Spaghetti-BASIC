import re
import sys
from pathlib import Path

# Regex to identify function or method definitions (excluding main, constructors, etc. heuristically)
func_re = re.compile(r'^\s*(?:[a-zA-Z_][\w:<>]*\s+)+([a-zA-Z_][\w]*)\s*\([^;]*\)\s*(\{|$)')

def find_functions(file_path):
    results = []
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        for line_num, line in enumerate(f, 1):
            match = func_re.match(line)
            if match:
                func_name = match.group(1)
                results.append((func_name, file_path.name, line_num))
    return results

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 xref_functions.py <file1.cpp> [file2.cpp ...]")
        sys.exit(1)

    all_results = []
    for file_arg in sys.argv[1:]:
        path = Path(file_arg)
        if path.is_file() and path.suffix in ['.cpp', '.h', '.hpp', '.cc']:
            all_results.extend(find_functions(path))
        else:
            print(f"Skipping invalid file: {file_arg}")

    print(f"{'Function':30} {'File':30} {'Line'}")
    print("=" * 70)
    for func, fname, lnum in sorted(all_results):
        print(f"{func:30} {fname:30} {lnum}")
