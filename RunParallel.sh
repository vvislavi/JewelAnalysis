function arRunSingle() {
    #aliroot -l -b Steer.C\(\"${1}\"\)
    #aliroot -l -b CheckFMD.C+\(\"${1}\"\)
    #root -l -b CalibrateV0.C+\(\"${1}\"\)
    root -l -b MakeCorrHist.C+\(\"${1}\"\)
}
function RunSingle() {
    IFS=$'\n' read -d '' -r -a lines < ${1}
    c=0
    totcnt=0
    PIDS=()
    for i in ${lines[@]}
    do
	while [ $c -eq $2 ]
	    do
	    brLoop=0
	    sleep 1 #give a short break, so that the script is not running full time
	    for j in "${!PIDS[@]}"
	    do
		pid="${PIDS[$j]}"
		ps --pid $pid > /dev/null
		if [ "$?" -eq 0 ]
		then
		    continue
		fi
		(( c-=1 ))
		unset "PIDS[$j]"
		echo "Killing a job with PID ${pid}. Last added job is ${i}" >> log
		brLoop=1
	    done
	    if [ $brLoop -eq 1 ]
	    then
		break
	    fi
	done
	arRunSingle $i &
        pid=$!
        PIDS+=("$pid")
        (( c+=1 ))
        (( totcnt+=1 ))
        echo "Added job no. ${totcnt} on run ${i}"
        echo "Added job ${i} with PID ${pid} (tot count ${totcnt})" >> log
    done
    echo "Waiting for the last batch to finish..."
    for pid in "${PIDS[@]}"
    do
	wait $pid
    done
    echo "All finished now!"
}
