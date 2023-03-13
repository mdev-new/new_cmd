include Lexer
include Token

module Parser = struct

type return_type = Number of int | String of string | None of unit
type node = unit -> return_type

type parser = {
	nodes: node list
}

let rec sortTokens tokens = tokens

let make_node = function
| Eof -> fun x -> Number 0
| Illegal -> fun x -> Number 0
| Ident a -> fun x -> String a
| Int a -> fun x -> Number a

let stringify = function
| Number a -> string_of_int a
| String a -> a

let make_nodes input =
	let toks = Lexer.generate_tokens input in
	let rec gen tokens list = begin
		match tokens with
		| [] -> list @ []
		| h :: t -> gen t ((make_node h) :: list)
	end
	in gen (List.rev (sortTokens toks)) []

let new_parser input : parser = {
	nodes = (make_nodes input)
}

end