# the focus lengths we are interested in
F100 = [200 105 60 50 35];
F50 = [105 60 35];

function rads = aov (focus_length)
  # half angle of view in rads
  w = atan (36 ./ (2 * focus_length));
  h = atan (24 ./ (2 * focus_length));
  rads = [w', h'];
endfunction

function degs = aovd (rads)
  degs = rad2deg (2 * rads);
endfunction

function sizes = dome (rads, dome_diagonal)
  sizes = tan (rads) * dome_diagonal;
endfunction

Rads = aov (F)
Degs = aovd (Rads)

csvwrite ('Angles of View.csv', [ F100' Degs ],           "precision", "%.2f");
csvwrite ('Sizes Dome 100.csv', [ F100' dome(Rads,100) ], "precision", "%.2f");

Rads = aov (F50)
Degs = aovd (Rads)

csvwrite ('Sizes Dome 50.csv',  [ F50' dome(Rads,50)  ], "precision", "%.2f");

