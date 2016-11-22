read command
params="$(sed 's_\("\)_\\\1_g' | sed 's/\(.*\)/"\1"/g' | awk 'BEGIN{FS="\n"; ORS="\0";}{print}')"
#params="$(sed 's_\("\)_\\\1_g' | sed 's/\(.*\)/"\1"/g' | awk '{printf "%s%s", sep,$1; sep="|"}' FS="\n")"
echo "$command $params"
