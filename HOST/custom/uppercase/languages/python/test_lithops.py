# *****************************************************************************
# *                            cloudFPGA
# *                Copyright 2016 -- 2022 IBM Corporation
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *     http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *----------------------------------------------------------------------------

import sys
import os

from multiprocessing import Pool
from lithops.multiprocessing import Pool
#import lithops


input = ["HelloWorld", "HowAreYouToday?", "HelloWorld", "HowAreYouToday?", "HelloWorld", "HowAreYdsf4f43f4rf4f4f4ouToday?"]
output = "111111111111111111111111111111111111111111111111"

def my_function(input):
    trieres_lib=os.environ['cFpRootDir'] + "/HOST/custom/uppercase/languages/python/build"
    sys.path.append(trieres_lib)
    import _trieres
    out, output = _trieres.uppercase("127.0.0.1", "2718", input, True)
    return output

with Pool() as pool:
    async_result = pool.map_async(my_function, input)
    try:
        result = async_result.get(timeout=30)
        print(result)
    except TimeoutError:
        print("Timed out!")


#if __name__ == '__main__':
#    fexec = lithops.FunctionExecutor()
#    fexec.map(my_function, input)
#    result = fexec.get_result()
#    print(result)
#    fexec.plot(dst='lithops-plots') 
