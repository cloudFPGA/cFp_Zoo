import sys
import os
import numpy as np
import QuantLib as ql
trieres_lib=os.environ['cFpRootDir'] + "HOST/vision/harris/languages/python/build"
sys.path.append(trieres_lib)

import _trieres

input = os.environ['cFpRootDir'] + "ROLE/vision/hls/harris/test/512x512.png"

out, output_image, output_points = _trieres.harris("localhost", "1234", input)

print("The output image file is " + output_image)

print("The output points file is " + output_points)
