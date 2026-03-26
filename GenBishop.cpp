static void genBishop(const Position *p,
                      int from,
                      bool white,
                      const int dirs[][2],  // bishop directions
                      int dcount,
                      Move *moves,
                      int *n)
{
    int r = from / 8;
    int f = from % 8;

    for (int di = 0; di < dcount; di++)
    {
        int df = dirs[di][0];
        int dr = dirs[di][1];

        int cr = r + dr;
        int cf = f + df;

        // Slide diagonally until blocked
        while (cr >= 0 && cr < 8 && cf >= 0 && cf < 8)
        {
            int to = cr * 8 + cf;
            char target = p->b[to];

            if (target == '.')
            {
                // Empty square
                moves[*n] = Move(from, to);
                (*n)++;
            }
            else
            {
                // Occupied square
                bool targetWhite = isWhitePiece(target);

                if (targetWhite != white)
                {
                    // Capture
                    moves[*n] = Move(from, to);
                    (*n)++;
                }

                // Stop sliding after any piece
                break;
            }

            cr += dr;
            cf += df;
        }
    }
}