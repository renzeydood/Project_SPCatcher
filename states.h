enum Mode
{
    FULLY_AUTO,  //Stops when a target amount of shiny Pokemon are caught
    SEARCH_ONLY, //Stops until a shiny Pokemon is encountered (Will not catch)
    BREEDING,    //For future implementation
};

struct Mode_Stuct {
    Mode id;
    String name;
} Mode_List[] = {
    {FULLY_AUTO, "FULLY_AUTO"},
    {SEARCH_ONLY, "SEARCH_ONLY"},
    {BREEDING, "BREEDING"}
};

enum State
{
    IDLE,
    SEARCHING,
    CATCHING,
    ERROR,
    RESET,
    SAVING,
    ESCAPING,
};

struct State_Stuct {
    State id;
    String name;
} State_List[] = {
    {IDLE, "IDLE"},
    {SEARCHING, "SEARCHING"},
    {CATCHING, "CATCHING"},
    {ERROR, "ERROR"},
    {RESET, "RESET"},
    {SAVING, "SAVING"},
    {ESCAPING, "ESCAPING"}
};

State strToState(String str);

State strToState(String str){
    for(int i = 0; i < sizeof(State_List)/sizeof(State_List[0]); i++){
        if (strcmp(str.c_str(), State_List[i].name.c_str()) == 0){
            return State_List[i].id;
        }
    }
    return IDLE;
}