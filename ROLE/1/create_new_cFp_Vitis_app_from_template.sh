#/bin/sh
# @author DID

template_kernel="harris"
new_kernel="safe"

function replace() {
    files="$(find -L "$1" -type f)";
    if [[ "$files" == "" ]]; then
        echo "No files";
        return 0;
    fi
    file_count=$(echo "$files" | wc -l)
    echo "Count: $file_count"
    echo "$files" | while read filen; do
        if [[ $filen =~ "hls_reports" ]] || [[ $filen =~ "build" ]] 
        then
           echo "Skipping auxililiary file " ${filen}
           continue
        else
           echo "Replacing in file " ${filen}
        fi        
        rpl ${template_kernel} ${new_kernel} ${filen}
    done
}


find ./ -type d -name ${template_kernel} -print 2>/dev/null | while read dir_src_kernel; do
    echo  "#####################################"
    parentdir="$(dirname "$dir_src_kernel")"
    newdir=${parentdir}"/"${new_kernel} 
    echo  "Copying dir: " ${dir_src_kernel} " to " ${newdir}
    cp -rf ${dir_src_kernel} ${newdir}
    echo "Replacing file names in directory " ${newdir}
    pattern="*"${template_kernel}"*"
    echo "Finding in " ${newdir} "for " ${pattern}
    find ${newdir} -type f -name ${pattern} | while read FILE ; do
	echo "ranamed harris to safe in " ${FILE}
	rename 's/harris/safe/g' ${FILE}
    done
    echo "Replacing recursively in the files of directory " ${newdir}
    replace ${newdir}
done 

