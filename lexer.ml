include Token

module Lexer = struct
	type lexer = {
		input : string;
		position : int;
		read_position : int;
		ch : char;
	}

	let null_byte = '\x00'

	let peek_char lexer =
		if lexer.read_position >= String.length(lexer.input)
		then null_byte
		else String.get lexer.input lexer.read_position

	let read_char lexer =
		let read_to_end = lexer.read_position >= String.length(lexer.input) in
		let new_ch = match read_to_end with
		| true -> null_byte
		| false -> String.get lexer.input lexer.read_position
		in {lexer with position = lexer.read_position; read_position = lexer.read_position + 1; ch = new_ch}

	let new_lexer input_string =
		let lexer = {
			input = input_string;
			position = 0;
			read_position = 0;
			ch = null_byte;
		} in
		read_char lexer

	let is_letter c = 'a' <= c && c <= 'z' || 'A' <= c  && c <= 'Z' || c == '_'
	let is_digit c = '0' <= c && c <= '9'

	let read_type fn lexer =
		let position = lexer.position in
		let rec read lex = if (fn lex.ch) then lex |> read_char |> read else lex in
		let updated_lex = read lexer in
		(updated_lex, (String.sub updated_lex.input position (updated_lex.read_position - position - 1)))

	let read_identifier = read_type is_letter
	let read_number = read_type is_digit

	let rec next_char lexer =
		let skip l = l |> read_char |> next_char in
		let double_read l = l |> read_char |> read_char in
		match (lexer.ch, peek_char lexer) with
		| ('!', '=') -> (double_read lexer, Token.NotEquals)
		| ('+', '=') -> (double_read lexer, Token.PlusEquals)
		| ('-', '=') -> (double_read lexer, Token.MinusEquals)
		| ('*', '=') -> (double_read lexer, Token.TimesEquals)
		| ('/', '=') -> (double_read lexer, Token.DivideEquals)
		| ('^', '=') -> (double_read lexer, Token.XorEquals)
		| ('|', '=') -> (double_read lexer, Token.OrEquals)
		| ('=', '=') -> (double_read lexer, Token.Equals)
		| ('&', '=') -> (double_read lexer, Token.AndEquals)
		| ('&', '&') -> (double_read lexer, Token.LogAnd)
		| ('>', '>') -> (double_read lexer, Token.AppendOut)
		| ('|', '|') -> (double_read lexer, Token.LogOr)
		| ('=', _) -> (read_char lexer, Token.Assign)
		| ('(', _) -> (read_char lexer, Token.LeftParen)
		| (')', _) -> (read_char lexer, Token.RightParen)
		| (',', _) -> (read_char lexer, Token.Comma)
		| ('!', _) -> (read_char lexer, Token.Bang)
		| ('+', _) -> (read_char lexer, Token.Plus)
		| ('-', _) -> (read_char lexer, Token.Minus)
		| ('*', _) -> (read_char lexer, Token.Asterisk)
		| ('/', _) -> (read_char lexer, Token.Slash)
		| ('^', _) -> (read_char lexer, Token.Xor)
		| ('|', _) -> (read_char lexer, Token.Or)
		| ('&', _) -> (read_char lexer, Token.And)
		| ('<', _) -> (read_char lexer, Token.PipeIn)
		| ('>', _) -> (read_char lexer, Token.PipeOut)
		| (' ', _) -> skip lexer
		| ('\t', _) -> skip lexer
		| ('\n', _) -> skip lexer
		| ('\r', _) -> skip lexer
		| ('\x00', _) -> (lexer, Token.Eof)
		| (c, _) ->
			if (is_letter c) then
				let (new_lex, tok_literal) =  read_identifier lexer in
				(new_lex, Token.lookupIdent tok_literal)
			else begin
				match is_digit c with
				| true ->
					let (new_lex, tok_literal) = read_number lexer in
					(new_lex, Token.Int (int_of_string tok_literal))
				| false -> (read_char lexer, Token.Illegal)
			end

	let generate_tokens input_string =
		let lexer = new_lexer input_string in
		let rec gen lxr tokens =
			match next_char lxr with
			| (_, Token.Eof) -> List.rev_append tokens [Token.Eof]
			| (l, tok) -> gen l (tok :: tokens)
		in gen lexer []
end