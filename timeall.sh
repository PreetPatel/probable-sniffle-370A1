#!/bin/bash

if [ $# -lt 1 ]; then
    printf "Please specify the number of elements to run merge_sort on\n"
    printf "Usage: ./timeall.sh <num_elements> <executions_per_task> [-c (clean before compile)]\n"
    exit 1
fi

if [ $1 = "-h" ]; then
    printf "Compiles and times all your tasks. Any tasks which don't compile will be ignored.\n"
    printf "For each task, the average time is calculated and put in the right-most column.\n"
    printf "Usage: ./timeall.sh <num_elements> <executions_per_task> [-c (clean before compile)]\n"
    exit 0
fi

if [ $# -lt 2 ]; then
    printf "Please specify the number of times to run each task\n"
    printf "Usage: ./timeall.sh <num_elements> <executions_per_task> [-c (clean before compile)]\n"
    exit 1
fi

# clean if parameter set
if [ $# -gt 2 -a $3 = "-c" ]; then
    for i in {1..9}; do
        if [ -e "a1.$i" ]; then
            rm "a1.$i"
        fi
    done
fi

for i in {1..9}; do
    if [ -e "a1.$i.c" ]; then
        cc -pthread -o "a1.$i" "a1.$i.c" -lm
        if [ $? -ne 0 ]; then
            printf "cc threw an error on task $i, ignoring. \n"
        fi
    fi
done

bonus=0

if [ -e "a1.bonus.c" ]; then
    cc -pthread -o "a1.bonus" "a1.bonus.c" -lm
    if [ $? -ne 0 ]; then
        printf "cc error trying to compile bonus task\n"
        bonus=1
    fi
else
    printf "bonus task not found\n"
    bonus=1
fi

echo "done compiling"

printf "assignment_task;" > timings.csv

for (( i=1; i<=$2; i++ )) do
    printf "time$i;" >> timings.csv
done

printf "average\n" >> timings.csv

printf " ---- starting timing ---- \n"

echo "" > output.log
echo "" > error_output.log

time_sum="0"

testTask() {
    success=0
    for (( j=1; j<=$2; j++ )) do
        printf " --- Task $1, execution $j --- \n" >> output.log
        printf " --- Task $1, execution $j --- \n" >> error_output.log

        time=$(TIMEFORMAT="%R"; { time ./a1.$1 $3 1>&3 2>&4; } 2>&1)
        local result=$?

        if [ $result -eq 139 ]; then
            printf "SEGFAULT;" >> timings.csv
            success=1
        elif [ $result -eq 0 ]; then
            time_sum=$(echo "$time_sum + $time" | bc -l)
            printf "$time;" >> timings.csv
        else
            printf "ERROR;" >> timings.csv
            success=1
        fi
            
        printf "\n\n" >> output.log
        printf "\n\n" >> error_output.log
        
    done
    return $success
}

exec 3>>output.log 4>>error_output.log

for i in {1..9}; do
    if [ -e a1.$i ]; then
        echo "  -- timing task $i -- "
        printf "$i;" >> timings.csv

        time_sum="0"
        testTask $i $2 $1
        if [ $? -eq 1 ]; then
            average="N/A"
        else
            average=$(echo "$time_sum / $2" | bc -l)
        fi

        printf "$average\n" >> timings.csv
    fi
done

if [ $bonus -eq 0 ]; then
    echo "  -- timing bonus task -- "
    printf "bonus;" >> timings.csv

    time_sum="0"
    testTask "bonus" $2 $1
    average=$(echo "$time_sum / $2" | bc -l)

    printf "$average\n" >> timings.csv
fi

exec 3>&- 4>&-

printf "done :)\n"
