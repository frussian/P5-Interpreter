program hello(input, output);
var
  i, l: integer;
  s, s2, s3, s4: set of 0..255;
  elem: 0..255;
  arr: array[1..3] of 0..255;
begin
  writeln('Hello', ord(true));
  read(l);
  for i := 1 to l do begin
    s := s + [i];
  end;
  writeln;

  writeln('difference');
  if 4 in s then writeln('4 in set');
  s := s - [4];
  if not (4 in s) then writeln('4 not in set');

  writeln;
  writeln('intersection');
  arr[1] := 1;
  arr[2] := 8;
  arr[3] := 35;
  for i := 1 to 3 do s2 := s2 + [arr[i]];
  s := s * s2;
  for i := 1 to 3 do begin
    if arr[i] in s then begin
      write(arr[i]:1);
      write(' ');
    end;
  end;
  writeln;
  writeln;
  writeln('union');

  s2 := [1, 2, 4, 123];
  s3 := [1, 2, 5, 123];
  s4 := s2 + s3;
  if 1 in s4 then writeln('1');
  if 4 in s4 then writeln('4');
  if 2 in s4 then writeln('2');
  if 5 in s4 then writeln('5');
  if 23 in s4 then writeln('23');
  if 123 in s4 then writeln('123');

end.
