program readfile(input, output, prd);
var
  c: char;
begin
  reset(prd);
  while not eof(prd) do
  begin
    if eoln(prd) then
    begin
      read(prd, c);
      writeln;
    end
    else
    begin
      read(prd, c);
      write(c);
    end;
  end;

end.
