open Unix

let port = 19
let tcp = ref false
let size = 72

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

let char_table = (string_init size (fun i -> (Char.chr (i + 33)))) ^ "\r\n"

let tcp_speak sock =
  Unix.listen sock 10;
    while true do
      let (fd,_) = accept sock in
      try
        while true do
          ignore (send fd char_table 0 (size + 2) []);
          incr_string char_table size;
        done;
      with Unix_error (e,_,_) when e = ECONNRESET -> close fd
    done;
    close (sock)

let udp_speak sock =
  let tmp = String.create 512 in
  while true do
    let (ret, addr) = recvfrom sock tmp 512 0 [] in
    if ret >= 0 then
      ignore (sendto sock char_table 0 (size + 2) [] addr);
    incr_string char_table size
  done

let usage () =
  print_endline
  ("This is chargen: the character generator protocol daemon\n"                  ^
   "Usage :     " ^ Sys.argv.(0) ^ "    [--tcp|--udp|-h]\n"                  ^
   "Options: \n"                                                             ^
   "    --tcp or -tcp   : Use tcp protocol (default if no option is given)\n"^
   "    --udp or -udp   : Use udp protocol\n"                                ^
   "    --help or -help : Show this help\n"                                  )

let treat_arg () =
  let len = Array.length (Sys.argv) in
  let rec aux = function
    "--udp" | "-udp" -> tcp := false
    | "--tcp" | "-tcp" -> tcp := true
    | "-h" | "--help" | "-help" -> usage (); exit 0
    | _ -> usage (); exit 1 in
  if len = 1 then tcp := true
  else if len = 2 then
    aux Sys.argv.(1)
  else
    (usage (); exit 1)

let daemonize () =
  print_endline "\n -- Daemon Chargen started --";
  let aux () = if (fork ()) = 0 then () else exit (0) in
  aux (); ignore (setsid ()); aux (); (*Forks the process *)
  ignore (umask(0)); chdir("/")

let _ =
  treat_arg ();
  daemonize ();
  try
    let sock = (socket PF_INET (if !tcp then SOCK_STREAM else SOCK_DGRAM) 0)
    and hostinfo = gethostbyname (gethostname ()) in
    let server_addr = hostinfo.Unix.h_addr_list.(0) in
    ignore (bind sock (ADDR_INET (server_addr, port)));
    if !tcp then
      tcp_speak sock
    else
      udp_speak sock

  with Unix_error (e,str1,str2) -> print_string ((error_message e) ^ "\n" ^ str1
  ^ "\n" ^ str2 ^ "\n")
  | e -> print_string ("fail: "^  (Printexc.to_string e))

