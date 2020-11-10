import sys
import os

trieres_lib=os.environ['cFpRootDir'] + "/HOST/languages/python/build"
sys.path.append(trieres_lib)

import _trieres

input = "HelloWorld"
output = "111111111"

out, output = _trieres.uppercase("localhost", "1234", input, True)

print(out)
print(output)
