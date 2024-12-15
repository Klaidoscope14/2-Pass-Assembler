
#include<iostream>
#include<vector>
#include<map>
#include<fstream>
#include<sstream>

using namespace std;

//stores information about lines in intermediate instructions
struct interm
{
    string label;
    string instr;
    string op;
    interm(string &a , string &b , string &c){
		label = a ;
		instr = b ;
		op = c ;
	}
};

class checker {
public:
    //check decimal
    bool isDecimal(string s)
    {
        if(s[0]=='0' || s.size()!=1){
            return 0;
        }
        for(int i=1; i=s.size()-1 ; i++)
        {
            if(s[i]>'0' && s[i]<'9'){
                continue;
            }else{
                return false;
            }
        }
        return true;
    }
    //check octal
    bool isOctal(string s)
    {
        if(s.size()<=1 || s[0]!='0'){
            return false;
        }
        int i=1;
        while(i<=s.size()-1)
        {
            if(s[i]>'0' && s[i]<'7'){
                continue;
            }else{
                return false;
            }
            i++;
        }
        return true;
    }
	//check hexadecimal
    bool isHexadecimal(string s)
    {
        if(s.size()<3 || s[0]!='0' || tolower(s[1])!='x')
            return false;
        int i=2;
        while(i<=s.size()-1)
        {
            if((s[i]<'0' || s[i]>'9') && (tolower(s[i])<'a' || tolower(s[i])>'z')){
                return false;
            }
            i++;
        }
        return true;
    }
};
checker check;

map <string, pair<string, int> > OpTable;        //stores mnemonics and their information <mnemonic, <opcode, operand needed>>
string fileName = "";
string InputFile = "input.asm";
vector<string> CleanedInstr;                //cleaned instructions
vector<pair<int, string>> Errors;           //errors
vector<pair<int, string>> warning;          //warnings
vector<pair<int, interm>> intermInstr;      //intermediate instructions
map<string, pair<int,int>> labelTable;      //stores information about labels <label, <pc, line on which declared>>
map<string, pair<int, int>> usedLabels;     //labels used in operands <label, <line, 
vector<pair<int , interm>> Instr;           //instruction for pass2
map<string, string> SETvalue;               //info for SET instr <label, operand> 
vector<string> mCode;                       //machine code
vector<string> lFile;                       //listing files

int pc = 0;

//generate log file
bool writeErrors(){
    sort(Errors.begin(), Errors.end());
    if (!Errors.size())
    {
        cout<<"Assembled Succesfully\n";
        fstream file;
        file.open(fileName+".log", ios_base::out);
        file<<"No Errors. \n";
        if (warning.size()>0)
        {
            file<<"Warnings : \n";
            for (auto it = warning.begin(); it!=warning.end(); it++)
            {
                file<<"Line "<<it->first<<" : "<<it->second<<endl;
            }
        }
        file.close();
        cout<<"Log file generated. \n";
        return false;
    }
    else
    {
        cout<<"Failed to Assemble\n";
        fstream file;
        file.open(fileName+".log", ios_base::out);
        file<<"Errors : \n";
        for (auto it= Errors.begin(); it!= Errors.end(); it++)
        {
            file<<"Line "<<it->first<<" : "<<it->second<<endl;
        }
        if (warning.size())
        {
            file<<"Warnings : \n";
            for (auto it= warning.begin(); it!=warning.end(); it++)
            {
                file<<"Line "<<it->first<<" : "<<it->second<<endl;
            }
        }
        file.close();
        cout<<"Log file generated. \n";
        return true;
        
    }
    
    return false;
}

//generate listing file
void writeListingFile(){
    ofstream writeList(fileName+".lst");
    for (auto it=lFile.begin(); it!=lFile.end(); it++)
    {
        writeList<<*it<<endl;
    }
    writeList.close();
    cout<<"Listing File created. \n";
}

//convert octal to decimal
string oct2Dec(string s, int sign){
    return to_string(sign*stoul(s,nullptr,8));
}

//convert hexadecimal to decimal
string hex2Dec(string s){
    int ans=0,pow=1;
    int i=s.size()-1;
    while(i>=0)
    {
        if((s[i]<'0' || s[i]> '9')){
            ans+=(pow* ((tolower(s[i])-'a') +10));
        }
        else{
            ans+=(pow*(s[i]-'0'));
        }
        pow <<= 4;
        i--;
    }
    return to_string(ans);
}

//convert decimal to hexadecimal
string dec2hex(int number){
    unsigned int num = number;
    string ans = "";
    for (int i = 0; i < 8; i++)
    {
        int remainder = num%16;
        if(remainder>9){
            ans+= char(remainder - 10 + 'A');
        }else{
            ans+=char(remainder+'0');
        }
        num >>= 4;
    }
    return string(ans.rbegin(), ans.rend());
}

//creates the Opcode Table
void Optable(){
    // 1 means no operand needed
    // 2 means a value is needed 
    // 3 means offset is needed
OpTable["data"] = make_pair("", 2);
OpTable["ldc"] = make_pair("00", 2);
OpTable["adc"] = make_pair("01", 2);
OpTable["ldl"] = make_pair("02", 3);
OpTable["stl"] = make_pair("03", 3);
OpTable["ldnl"] = make_pair("04", 3);
OpTable["stnl"] = make_pair("05", 3);
OpTable["add"] = make_pair("06", 1);
OpTable["sub"] = make_pair("07", 1);
OpTable["shl"] = make_pair("08", 1);
OpTable["shr"] = make_pair("09", 1);
OpTable["adj"] = make_pair("0A", 2);
OpTable["a2sp"] = make_pair("0B", 1);
OpTable["sp2a"] = make_pair("0C", 1);
OpTable["call"] = make_pair("0D", 3);
OpTable["return"] = make_pair("0E", 1);
OpTable["brz"] = make_pair("0F", 3);
OpTable["brlz"] = make_pair("10", 3);
OpTable["br"] = make_pair("11", 3);
OpTable["HALT"] = make_pair("12", 1);
OpTable["SET"] = make_pair("", 2);

}

//function to remove any white spaces and comments
string cleanInstr(string s){
    string out ;
    int i=0;
    while(i <= 1){
		reverse(s.begin(), s.end());
		while(s.back() == ' ' or s.back() == '\t'){
			s.pop_back();
		}
        i++;
	}
    i = 0;
    bool comma=0;
	for(; i < (int)s.size() && !comma; i++){
		if (s[i] == ';')
        {
            comma=1;
            break;
        }
        if(s[i] == ':'){
			out.push_back(s[i]);
			if(i == s.size() - 1 or s[i + 1] != ' ')
				out.push_back(' ');
			continue;
		}
		out.push_back(s[i]) ;
	}
	return out ; 
}


//seperate the line into label, instruction and operand
void seperateInstr(string &label, string &instr, string &op, int line, string &cur){
    int i = 0;
    auto labelPresent = 0;
    while (i <= cur.size()-1 && !labelPresent)
    {
        if (cur[i]==':')
        {
            labelPresent = 1;
            label = string(cur.begin(), cur.begin()+i);
        }
        i++;
    }
    if (!labelPresent)
    {
        i = 0;
    }
    while(i <= cur.size()-1 && (cur[i] == ' ' || cur[i] == '\t')){
        i++ ;
    }
    while(i <= cur.size()-1 && (cur[i] != ' ' && cur[i] != '\t')){
		instr.push_back(cur[i]);
		i++;
	}
	while(i <= cur.size()-1 && (cur[i] == ' ' || cur[i] == '\t')){
        i++ ;
    }
	while(i <= cur.size()-1 && !(cur[i] == ' ' || cur[i] == '\t' || cur[i] == ',')){
		op.push_back(cur[i]);
		i++ ;
	}
	while(i <= cur.size()-1 && (cur[i] == ' ' || cur[i] == '\t' || cur[i] == ',')){
        i++ ;
    }
	if(i < cur.size() && (cur[i] != ' ' && cur[i] != '\t')){
		Errors.push_back({line , "Extra operand present"}) ;
	}
}

//check if label is valid
bool correctLabel(string label){
    if (!isalpha(label[0]))
    {
        return false;
    }
    for (int i = 0; i < label.size(); i++)
        {
            if (isalnum(label[i]) || label[i]=='_')
            {
                continue;
            }else{
                return false;
            }
        }
    return true;
}

//process the operand and convert it to decimal
string solveOperand(string operand, int sign){
    if (check.isOctal(operand))
    {
        return oct2Dec(string(operand.begin()+1, operand.end()), sign);
    }
    if (check.isHexadecimal(operand))
    {
        return to_string(stoi(hex2Dec(string(operand.begin()+2, operand.end())))*sign);
    }
    bool ans = true;
    int i=0;
    while(i < operand.size())
    {
        ((operand[i]<'0' || operand[i]> '9') ? ans &= false : ans &= true);
        i++;
    }
    if (ans)
    {
        return to_string(sign*stoi(operand));
    }
    return "false";
}

//check if operand is valid and process it
bool checkOperand(string op, string &newOperand, int line){
    string s;
    int sign = 1;
    if (correctLabel(op)==1)
    {
        usedLabels[op] = {line, 1};
        newOperand = op;
        return true;
    }
    if (op[0]=='-')
    {
        sign = -1;
    }
    if (op[0]!='+' && op[0]!='-')
    {
        s = solveOperand(op, sign);
    }
    else
    {
        s = solveOperand(string(op.begin()+1, op.end()), sign);
    }
    if (s == "false")
    {
        return false;
    }
    newOperand = s;
    return true;
}


//check if the labels, instructions and operands are valid and also process them
void checkValidity(){
    bool haltPresent = false;
    for ( auto it= intermInstr.begin(); it!=intermInstr.end(); it++)
    {
        int line;
        line = it->first;
        string label, instr, op;
        label = it->second.label;
        instr = it->second.instr;
        op =it->second.op;
        auto labelValid = false;
        auto operandNeeded = false;
        int cnt = 0;
        string temporay = "";
        string validLabel = "";
        int flag = 0;
        int flag1 = 0;
        if (label.empty()==0)                 //check if label is valid
        {
            if (correctLabel(label))
            {
                if (labelTable.count(label) > 0)
                {
                    Errors.push_back({line, "Duplicate definition of Label : \""+label+"\""});
                }
                else
                {
                    labelTable[label] = {pc, line};         //put declared label in table
                    labelValid = true;
                    validLabel = label;
                }
                
            }
            else{
                Errors.push_back({line, "Invalid Label Name"});
            }
        } 
        if (instr.empty()==0)         //check if instruction is valid
        {
            if (OpTable.count(instr)>0)
            {
                if (OpTable[instr].second >= 2)
                {
                    operandNeeded = true;
                }
                if (instr == "SET")   //check for SET instr
                {
                    if (label.empty()==1)
                    {
                        Errors.push_back({line, "Missing label for SET"});
                    }
                    if (checkOperand(op, temporay, line)==0 || isalpha(op[0]))
                    {
                        Errors.push_back({line, "Operand for SET is not a valid value"});
                        flag1 = 1;
                    }
                    if (op.empty()==1)
                    {
                        Errors.push_back({line, "Missing Operand"});
                    }
                    if(!(label.empty() || flag1 || op.empty())){
                        SETvalue[label] = op;
                        flag = 1;
                    }
                }
                if (instr == "HALT")
                {
                    haltPresent = true;
                }
                if (instr == "data" && labelValid)  //check for data instr
                {
                    usedLabels[label] = {line, 1};
                }  
                cnt = 1;
            }else{
                Errors.push_back({line, "Invalid Mnemonic"});
                
            }
        }
        string newOperand = "";
        if (op.empty())  //check operand
        {
            if (operandNeeded){
            Errors.push_back({line, "Missing Operand"});
            }
            
            
        }else {
            if (operandNeeded)
            {
                if (!checkOperand(op, newOperand, line))   //process and check operand 
                {
                    Errors.push_back({line, "Invalid Operand"});
                }
            }
            else{
                Errors.push_back({line, "Unexpected Operand"});
                
                
            }
        }
        Instr.push_back({pc, interm(label, instr, newOperand)});  
        pc = pc + cnt;          //update pc
    }
    for (auto it= labelTable.begin(); it!= labelTable.end(); it++)
    {
        string label =it->first;
        int line =it->second.second;
        if (usedLabels.count(label)==0)
        {
            string err="Label \""+label+"\" declared but not used";
            warning.push_back({line, err});
        }
    }
    for (auto it= usedLabels.begin(); it!=usedLabels.end(); it++)
    {
        string label =it->first;
        int line =it->second.first;
        if (labelTable.count(label)==0)
        {
            string err="No such label \""+label+"\" has been declared";
            Errors.push_back({line,err});
        }
        
    }
    if (haltPresent==0)
    {
        warning.push_back({(int)CleanedInstr.size()+1, "HALT instruction not found."});
    }
    
}

//generate machine code file
void writeMachineCode(){
    ofstream writeMCode;
    writeMCode.open(fileName+".o", ios::binary | ios::out);
    for (auto it= mCode.begin(); it!=mCode.end(); it++)
    {
        if ((*it).empty() or *it == "        "){
            continue;
        }
        string x = hex2Dec(*it);
        unsigned int y = (unsigned int)stoi(hex2Dec(*it));
        static_cast<int>(y);
        writeMCode.write((const char*)&y , sizeof(unsigned int));
    }
    writeMCode.close();
    cout<<"Machine code file generated. \n";
}

//pass1 of assembler
void firstPass(){
    ifstream in; 
    in.open(InputFile);     //open input file
    if (in.fail())
    {
        cout<<"Input file doesn't exist";
        exit(0);
    }
    string s;
    int line = 0;
    while (getline(in,s))
    {
        CleanedInstr.push_back(s);
        string cur =""; 
        cur = cleanInstr(s);
        line++;
        if(cur.length() == 0){
            continue;
        }
        string label, instr, op;
        label = "";
        instr = "";
        op = "";
        seperateInstr(label, instr, op, line, cur);  
        intermInstr.push_back({line, interm(label, instr, op)});   
    }
   checkValidity();    
}

//pass 2 of assembler
void pass2(){
    for (auto it= Instr.begin(); it!=Instr.end(); it++)
    {
        int prct =it->first;
        string label =it->second.label;
        string instr =it->second.instr;
        string operand =it->second.op;
        int type = -1;
        if (instr.empty()==0)
        {
            type = OpTable[instr].second;
        }
        string machineCode = "        ";
        if (type == 1) {
            machineCode = "000000" + OpTable[instr].first;
        } 
        else if (type == 2) {
            if (instr != "data" && instr != "SET") {
                string temporary = "";
                int ans = 0;
                if (labelTable.count(operand)) {
                    ans = labelTable[operand].first;
                } else {
                    ans = stoi(operand);
                }
                temporary = dec2hex(ans);
                temporary = string(temporary.begin()+2, temporary.end());
                if (SETvalue.count(operand)) {
                    temporary = dec2hex(stoi(SETvalue[operand]));
                    temporary = string(temporary.begin()+2, temporary.end());
                }
                machineCode = temporary + OpTable[instr].first;
            } else {
                machineCode = dec2hex(stoi(operand));
                
            }
        } 
        else if (type == 3) {
            int offset;
            if (labelTable.count(operand)) {
                if (!SETvalue.count(operand)) {
                    offset = labelTable[operand].first-(prct+1);
                } else {
                    offset = stoi(SETvalue[operand])-(prct+1);
                }
                
            } else {
                offset = stoi(operand);
            }
            machineCode = dec2hex(offset).substr(2) + OpTable[instr].first;
        }

        if (label.empty()==0)
        {
            label += ": ";
        }
        if (instr.empty()==0)
        {
            instr += " ";
        }
        if (machineCode != "        ")
        {
            auto programCount = dec2hex(prct);
            string listing = programCount+ " " + machineCode + " " + label + instr + operand;
            lFile.push_back(listing);
        }
        mCode.push_back(machineCode);
    }
}

int main(int argc, char* argv[]) //main driver function
{
    InputFile = argv[1];  //the input file
    fileName= InputFile;
    fileName = string(fileName.begin(), fileName.end()-4);
    Optable();                       //initialize the Opcode Table;
    firstPass();                     // first pass 
    if(writeErrors()){
        return 0 ;                  // if errors return 
    }
   pass2();                         //pass 2
   writeListingFile();              // create the list file
   writeMachineCode();              // create the machine code file

   return 0;
}