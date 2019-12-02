#!/bin/bash
rustc main.rs

# Optional parameter ${6:-1} - to choose function fx
for i in {1..10}; do ./main | tee -a temp_results.txt; done
# for i in {1..10}; do ./quadratura $1 $2 $3 $4 $5 ${6:-1} >> temp_results.txt; done

# ignore whats before 'tempo' and save it in another file
cat temp_results.txt | grep -o 'tempo.*' > temp_tempos.txt

# sort it
sort temp_tempos.txt -o temp_tempos.txt

# get median
#sed -n '5p' < temp.txt
printf "\n\t" && echo $(cat temp_results.txt | grep -o -P 'Area somada .* (?=\|)' | head -1)
printf "\tMediana: " && echo $(sed -n '5p' < temp_tempos.txt)s && printf "\n"

# delete temp files
rm temp_tempos.txt temp_results.txt
