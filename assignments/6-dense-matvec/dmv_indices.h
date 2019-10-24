int GetOrigin(int numRows, int numCols, int row, int col)
{
    return row * numCols + col;
}
int GetDest(int numRows, int numCols, int row, int col)
{
    int curr_rank = col * numRows + row;
    int rrow, rcol;
    rrow = curr_rank / numCols;
    rcol = curr_rank % numCols;
    return rcol * numRows + rrow;
}
