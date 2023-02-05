include Interpreter
include Lexer
include Parser

(*let intp = new_interpreter *)
(*
let l = Lexer.generate_tokens "if 1 LEQ 2";;
print_endline (String.concat " " (List.map Token.token_to_string l));;
*)

let p = Parser.new_parser "hello 222";;
Interpreter.interpret p.nodes

(*print_endline (String.concat " " (List.map string_of_int p.nodes));;*)