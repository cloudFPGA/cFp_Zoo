#/bin/sh
# @author DID
# @brief A script that creates the files and directories for a new Vitis kernel based on a previous 
#        which acts as a template.
# @note Change the following variables and launch the script with no arguments in this folder.


# Choose the kernel which will be the "template" from which the new directries/files will be generated
template_kernel="harris"
template_kernel2="Harris"
template_kernel3="HARRIS"

# Choose the name of the new kernel/files/dirs
new_kernel="newharrisfortest"
new_kernel2="Newharrisfortest"
new_kernel3="NEWHARRISFORTEST"

function replace() {
    files="$(find -L "$1" -type f)";
    if [[ "$files" == "" ]]; then
        echo "No files";
        return 0;
    fi
    file_count=$(echo "$files" | wc -l)
    echo "Count: $file_count"
    echo "$files" | while read filen; do
        if [[ $filen =~ "hls_reports" ]] || [[ $filen =~ "build" ]] || [[ $filen =~ "Vitis_Libraries" ]]  || [[ $filen =~ "cFDK" ]] || [[ $filen =~ "hlslib" ]] 
        then
           echo "Skipping auxililiary file " ${filen}
           continue
        else
           echo "Replacing in file " ${filen}
        fi        
        rpl ${template_kernel} ${new_kernel} ${filen}
        rpl ${template_kernel2} ${new_kernel2} ${filen}
        rpl ${template_kernel3} ${new_kernel3} ${filen}
    done
}


find ./ -type d -name ${template_kernel} -print 2>/dev/null | while read dir_src_kernel; do
    echo  "#####################################"
    if [[ $dir_src_kernel =~ "Vitis_Libraries" ]] || [[ $filen =~ "build" ]]
    then
       echo "Skipping auxililiary dir " ${dir_src_kernel}
       continue
    else
       echo "Replacing in directory " ${dir_src_kernel}
    fi            
    parentdir="$(dirname "$dir_src_kernel")"
    newdir=${parentdir}"/"${new_kernel} 
    echo  "Copying dir: " ${dir_src_kernel} " to " ${newdir}
    cp -rf ${dir_src_kernel} ${newdir}
    echo "Replacing file names in directory " ${newdir}
    pattern="*"${template_kernel}"*"
    echo "Finding in "${newdir}" for "${pattern}
    find ${newdir} -type f -name "${pattern}" | while read FILE ; do
        echo "Renaming ${template_kernel} to ${new_kernel} in " ${FILE}
        # rename 's/'${template_kernel}'/'${new_kernel}'/g' ${FILE} # This is the perl command rename
        rename ${template_kernel} ${new_kernel} ${FILE} # This is the bash command rename
    done
    echo "Replacing recursively in the files of directory " ${newdir}
    replace ${newdir}
done 

