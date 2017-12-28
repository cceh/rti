function p = bar3xy(hax, M, width = 0.1)

    # usage: bar3xy (hax, M, width)
    #
    # Draw a 3D bar graph with arbitrary bar positions.
    #
    # hax   - Axes
    # M     - Matrix of [x, y, height; ... ]
    # width - bar width (defaults to 0.1)
    #
    # Adapted from this answer by user Amro:
    #   https://stackoverflow.com/questions/24180890

    % sanity checks
    assert (ismatrix (M), 'Matrix expected.')
    [ny,nx] = size (M);
    assert (nx == 3, 'Matrix should be of size 3 x N');
    hw = width / 2;

    % first we build a "template" for the 8 vertices of a 3D bar
    % initially centered at position (0,0) with width=width and height=0
    [X,Y,Z] = ndgrid([-hw hw], [-hw hw], [0 0]);
    v = [X(:) Y(:) Z(:)];
    % replicate vertices of "template" to form ny bars
    v = v + permute(M,[3 2 1]);
    v = reshape(permute(v, [2 1 3]), 3, []).';
    % set the 4 upper vertices to the correct height
    v(:,3) = kron(M(:,3), [0,0,0,0,1,1,1,1]');

    % now we build a "template" for the 6 faces of a 3D bar
    % these are indices into the vertex matrix
    f = [
        1 2 4 3 ; % bottom
        5 6 8 7 ; % top
        1 2 6 5 ; % front
        3 4 8 7 ; % back
        1 5 7 3 ; % left
        2 6 8 4   % right
    ];
    % replicate faces of "template" to form ny bars
    f = f + permute (0:8:8 * (ny-1), [1 3 2]);
    f = reshape (permute (f, [2 1 3]), 4, []).';

    % colors
    c = [
      0 0 1
      1 1 0
      0 1 0
      1 0 0
      1 0 1
      0 1 1
      ];

    % plot
    % draw patch specified by faces/vertices
    % (we use a solid color for all faces)
    p = patch ('Faces', f, 'Vertices', v, ...
       "FaceColor", "flat", 'FaceVertexCData', repmat (c, ny, 1), ...
       'EdgeColor', 'k', 'Parent', hax);
end
