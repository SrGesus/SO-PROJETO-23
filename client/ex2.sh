# Example commands
command1="./client ../req_pipe1 ../res_pipe1 ../reg_pipe ../jobs/a.jobs"
command2="./client ../req_pipe ../res_pipe ../reg_pipe ../jobs/test.jobs"
command3="./client ../req_pipe2 ../res_pipe2 ../reg_pipe ../jobs/b.jobs"
command4="./client ../req_pipe3 ../res_pipe3 ../reg_pipe ../jobs/c.jobs"
command5="./client ../req_pipe4 ../res_pipe4 ../reg_pipe ../jobs/d.jobs"
command6="./client ../req_pipe5 ../res_pipe5 ../reg_pipe ../jobs/e.jobs"

chmod +x "$0"
# Run commands concurrently
echo -e "Running commands concurrently:"
($command1) &
($command2) &
($command3) &
($command4) &
($command5) &
($command6)

# Wait for all background processes to finish
wait

