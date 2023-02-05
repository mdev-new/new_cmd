module Token = struct
	type token_type =
		| Illegal
		| Eof
		(* Identifiers and literals *)
		| Ident of string
		| String of string
		| Int of int
		(* Operators *)
		| And
		| LogAnd
		| Or
		| LogOr
		| Xor
		| BinNot
		| Bang
		| PipeIn (* < *)
		| PipeOut (* > *)
		| AppendOut (* >> *)
		| Plus
		| Minus
		| Asterisk
		| Slash
		| Assign
		| Equals
		| NotEquals
		(* Assignment operators *)
		| PlusEquals
		| MinusEquals
		| TimesEquals
		| DivideEquals
		| XorEquals
		| OrEquals
		| AndEquals
		(* -- Delimiters *)
		| Comma
		| LeftParen
		| RightParen
		(* -- Keywords *)
		| In
		| Do
		| For
		| If
		| Else
		| Neq
		| Equ
		| Lss
		| Gtr
		| Leq
		| Geq

	type token = {
		t_type : token_type;
		literal : string;
	}

	let token_to_string = function
	| Illegal -> "Illegal"
	| Eof -> "Eof"
	| Ident a -> "Ident(" ^ a ^ ")"
	| String a -> "String(" ^ a ^ ")"
	| Int a -> "Int(" ^ (string_of_int a) ^ ")"
	(* Operators *)
	| And -> "And"
	| LogAnd -> "LogAnd"
	| Or -> "Or"
	| LogOr -> "LogOr"
	| Xor -> "Xor"
	| BinNot -> "BinNot"
	| Plus -> "Plus"
	| Minus -> "Minus"
	| Bang -> "Bang"
	| Asterisk -> "Asterisk"
	| Slash -> "Slash"
	| Assign -> "Assign"
	| Equals -> "Equals"
	| NotEquals -> "NotEquals"
	| PipeIn -> "PipeIn" (* < *)
	| PipeOut -> "PipeOut" (* > *)
	| AppendOut -> "DblPipeOut" (* >> *)
	(* Assignment operators *)
	| PlusEquals -> "PlusEquals"
	| MinusEquals -> "MinusEquals"
	| TimesEquals -> "TimesEquals"
	| DivideEquals -> "DivideEquals"
	| XorEquals -> "XorEquals"
	| OrEquals -> "OrEquals"
	| AndEquals -> "AndEquals"
	(* -- Delimiters *)
	| Comma -> "Comma"
	| LeftParen -> "LeftParen"
	| RightParen -> "RightParen"
	(* -- Keywords *)
	| In -> "In"
	| Do -> "Do"
	| For -> "For"
	| If -> "If"
	| Else -> "Else"
	| Neq -> "Neq"
	| Equ -> "Equ"
	| Lss -> "Lss"
	| Gtr -> "Gtr"
	| Leq -> "Leq"
	| Geq -> "Geq"

	(* TODO, convert from association list to map *)
	let string_to_token = [
		"if", If;
		"else", Else;
		"for", For;
		"do", Do;
		"in", In;
		"equ", Equ;
		"neq", Neq;
		"gtr", Gtr;
		"lss", Lss;
		"leq", Leq;
		"geq", Geq;
	]

	let lookupIdent str = try
		let s = String.lowercase_ascii str in List.assoc s string_to_token
		with Not_found -> Ident str

	let tokens_eq tok_a tok_b = match (tok_a, tok_b) with
	| Ident a, Ident b -> a = b
	| tok_a, tok_b -> tok_a = tok_b

	let new_token tok_type ch = {
		t_type = tok_type;
		literal = ch;
	}
end