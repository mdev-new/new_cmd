include Parser

module Interpreter = struct

let stringify = function
| Parser.Number a -> string_of_int a
| Parser.String a -> a

let rec interpret = function
| [] -> begin end
| h :: t -> begin print_endline (stringify (h())); interpret t; end

end