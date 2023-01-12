program hello(input, output);
var
  i, l: integer;
begin
  writeln('Hello', ord(true));
  read(l);
  for i := 1 to l do begin
    writeln(i);
  end
end.
