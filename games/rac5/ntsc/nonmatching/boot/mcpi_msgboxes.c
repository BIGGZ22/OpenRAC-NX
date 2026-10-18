/* Candidate only: the full-program build and compiler profile are not qualified. */
extern int my_msg_box_id;
extern int their_msg_box_id;

void MCPI_SetMyMsgBoxId__Fi(int value) { my_msg_box_id = value; }
void MCPI_SetTheirMsgBoxId__Fi(int value) { their_msg_box_id = value; }
int MCPI_GetMyMsgBoxId__Fv(void) { return my_msg_box_id; }
int MCPI_GetTheirMsgBoxId__Fv(void) { return their_msg_box_id; }
