open Unix

let port = 1664

let string_init size f =
  let str = String.make size 'a' in
  for i = 0 to size - 1 do
    String.set str i (f i)
  done; str

let incr_string str size =
  for i = 0 to size - 1 do
    let int_val = Char.code str.[i] in
    if (int_val = 126) then
      String.set str i ' '
    else
      String.set str i (Char.chr (int_val + 1))
  done

let communicate sock =
  let size = 72 in
  let char_table = string_init size (fun i -> (Char.chr (i + 33))) in
  let rec echo_loop () =
    ignore (send sock (char_table ^ "\r\n") 0 (size + 2) []);
      incr_string char_table size;
      echo_loop () in
  echo_loop ()

let daemonize () =
  let aux () = if (fork ()) = 0 then () else exit (0) in
  aux (); ignore (setsid ()); aux (); (*Forks the process *)
  ignore (umask(0)); chdir("/")

let _ =
  daemonize ();
  try
    let sock = (socket PF_INET SOCK_STREAM 0) in
    let hostinfo = gethostbyname (gethostname ()) in
    let server_addr = hostinfo.Unix.h_addr_list.(0) in
    ignore (bind sock (ADDR_INET (server_addr, port)));
    Unix.listen sock 10;
    while true do
      let (fd,_) = accept sock in
      try
        communicate fd
      with Unix_error (e,_,_) when e = ECONNRESET -> close fd
    done
  with Unix_error (e,str1,str2) -> print_string ((error_message e) ^ "\n" ^ str1
  ^ "\n" ^ str2 ^ "\n")
  | e -> print_string (Printexc.to_string e)

