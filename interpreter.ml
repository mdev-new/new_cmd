include Parser

module Interpreter = struct

let interpret_node = function
| Node.Call arg -> 0
| Node.Comment arg -> 0

let rec interpret = function
| [] -> begin (* empty block *) end
| head :: tail ->
  begin
    print_endline (Parser.stringify (head()));
    interpret tail;
  end

end