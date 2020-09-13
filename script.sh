#Store array of arguments into an array.
args=("$@") 
#Check for exactly 4 arguments.
if [ "${#args[@]}" != "4" ]; then
    printf "You have to insert exactly these 4 arguments...\nNum_of_Peers Num_of_Entries Ratio(Readers-Writers) Iteration_Times\n"; exit 1
fi
make
printf "\n"
./main $1 $2 $3 $4
make clean