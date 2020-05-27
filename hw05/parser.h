

#ifndef HW3_PARSER_H
#define HW3_PARSER_H
#define YYSTYPE STEntry*
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stack>
#include <iostream>
#include <string>
#include <vector>
#include "hw3_output.hpp"
#include "bp.hpp"

using namespace std;

class STEntry{
public:
    int _offset;
    string _name;
    string _type;
    bool _isFunc;
    bool _isEnumType;
    bool _isId;

    string _enumVal;

    vector<pair<int,BranchLabelIndex>> _truelist;
    vector<pair<int,BranchLabelIndex>> _falselist;
    vector<pair<int,BranchLabelIndex>> _breaklist;
    vector<pair<int,BranchLabelIndex>> _continuelist;
    vector<pair<int,BranchLabelIndex>> _nextlist;
    string _place;
    int _evalue;
    //string _lbl;
    bool _is_return_statement;


    STEntry():_offset(0),_name(""),_type("n/a"),_isFunc(false),_isEnumType(false),_isId(false)
                                            ,_place(""),_evalue(0),_is_return_statement(false),_enumVal("nothing"){
        _breaklist = vector<pair<int,BranchLabelIndex>>();
        _continuelist = vector<pair<int,BranchLabelIndex>>();
        _nextlist = vector<pair<int,BranchLabelIndex>>();
        _truelist = vector<pair<int,BranchLabelIndex>>();
        _falselist = vector<pair<int,BranchLabelIndex>>();
    }
    STEntry(const string& name,const string& type = "n/a"){
        _name = name;
        _type = type;
        _offset = 0;
        _isFunc = false;
        _isEnumType = false;
        _isId=false;
        _evalue=0;
        _place="";
        _is_return_statement=false;
        _enumVal="nothing";
        _breaklist = vector<pair<int,BranchLabelIndex>>();
        _continuelist = vector<pair<int,BranchLabelIndex>>();
        _nextlist = vector<pair<int,BranchLabelIndex>>();
        _truelist = vector<pair<int,BranchLabelIndex>>();
        _falselist = vector<pair<int,BranchLabelIndex>>();
    }
    virtual ~STEntry()= default;

};

class Explist : public STEntry{
public:
    vector<STEntry*> _exps;
    Explist(){}
    Explist(Explist &e) : _exps(e._exps) {}
};

class Function : public STEntry{
public:
    string _funcType;
    vector<string> _argTypes;
    string _varReg;
    //vector<string> _argTypesForPrint;

    Function(const string& name,const string& ret_type,const string& arg){
        _argTypes.push_back(arg);
        _funcType = output::makeFunctionType(ret_type,_argTypes);
        _name = name;
        _type = ret_type;
		_isFunc = true;
		_varReg = "n/a";


    }
    Function(const string& name,const string& ret_type, vector<string>& args){
		_argTypes = args;
        _funcType = output::makeFunctionType(ret_type,args);
        _name = name;
        _type = ret_type;
		_isFunc = true;
        _varReg = "n/a";
    }

    void checkArguments(Explist* explst,int line) {

        int argsNum = _argTypes.size();
        if(explst == nullptr){
            if (argsNum != 0) {
                output::errorPrototypeMismatch(line, _name, _argTypes);
                exit(0);
            }
			return;
        }

        if (argsNum != explst->_exps.size()) {
            output::errorPrototypeMismatch(line, _name, _argTypes);
            exit(0);
        }

        for (int i = 0; i < argsNum; i++) {

			
            if (_argTypes[argsNum - i - 1] != explst->_exps[i]->_type) {
                if (_argTypes[argsNum - i - 1] == "INT" &&
                    explst->_exps[i]->_type == "BYTE")
                { /* Implicit covnersion */ }
                else if(_argTypes[argsNum - i - 1].substr(_argTypes[argsNum - i - 1].find(' '),_argTypes[argsNum - i - 1].length()) == explst->_exps[i]->_type.substr(explst->_exps[i]->_type.find(' '),explst->_exps[i]->_type.length()))
                { /* Enums, it's ok. */ }
                else {
                    output::errorPrototypeMismatch(line, _name, _argTypes);
                    exit(0);
                }
            }
        }
    }

    string llvmFormat(){
        std::locale loci;
        string args = "";
        string res = "";

        if(_argTypes.size() > 0){
            for(string r : _argTypes){
                args = args+"i32 ,"; /* TODO: Actual params?.. */
            }
            args = args.substr(0,args.length()-1);
        }
        string rtype;
        if (_type == "VOID" ) rtype = "void";
        else rtype = "i32";
        res = rtype+" @"+_name+"("+args+")";
        return res;
    }
};

class Formalslist : public STEntry{
public:
    vector<string> _params;
    Formalslist() = default;

    void pushArg(string a){
        _params.insert(_params.begin(),a);
    }
};

class EnumType : public STEntry{
public:
    vector<string> _enumerators;
    string _id;
    //Declaration:  enum _id _name := _enumerator[i]
    EnumType(string id):STEntry(id,"enum "+id),_id(id){
        _isEnumType = true;
    }
    EnumType(string id,vector<string>& enumerators):STEntry(id,"enum "+id),_id(id){
        _enumerators = vector<string>(enumerators);
        _isEnumType = true;
    }

    bool containsEnumerator(string e){
        for(string t : _enumerators){
            if(t==e)
                return true;
        }
        return false;
    }
    int getEnumVal(string e){
        for(int i = 0 ; i < _enumerators.size(); i++){/* TODO: Might need to reversed!! */
            if(e == _enumerators[i]) return i;
        }
        return -1;
    }
};

class EnumeratorList : public STEntry{
public:
    vector<string> _enumerators;
    EnumeratorList() = default;

    void push(string e,int line){
        for(string en : _enumerators){
            if(en == e){
                output::errorDef(line,e);
                exit(0);
            }
        }
        _enumerators.push_back(e);
    }
};

class Num : public STEntry{

public:
    int _val;
    string _sval;
    Num(){};
    Num( string type, string val ="0"):STEntry("n/a",type),_sval(val){
        _val = atoi(val.c_str());
    };
    Num(Num& num): STEntry(num._sval,num._type), _sval(num._sval), _val(num._val) {};
};
class Bool : public STEntry{
public:
    Bool():STEntry("n/a","BOOL"){};
};
class Binop : public STEntry{
public:
    STEntry *_e1, *_e2;
    Binop(STEntry* e1,STEntry* e2,int line){
        if((e1->_type == "BYTE" || e1->_type == "INT") && (e2->_type == "BYTE" || e2->_type == "INT")){
            if(e1->_type == "INT" || e2->_type == "INT") _type = "INT";
            else _type = "BYTE";
            _e1 = e1;
            _e2 = e2;
        }
        else{
            output::errorMismatch(line);
            exit(0);
        }
    }
};
class Relop : public STEntry{
public:
    STEntry *_e1, *_e2;
    Relop(STEntry* e1, STEntry* e2,int line){
        if(e1->_type == "BOOL" && e1->_type =="BOOL"){}
            /* All is good man */
            else
        if((e1->_type != "INT" && e1->_type != "BYTE") ||
           (e2->_type != "INT" && e2->_type != "BYTE") ){
            output::errorMismatch(line);
            exit(0);
        }
        _e1 = e1;
        _e2 = e2;
        _type = "BOOL";
        _name = "n/a";
    }
};
class LogicOp : public STEntry{
public:
    STEntry *_e1, *_e2;
    LogicOp(STEntry* ex1,STEntry* ex2,int line){
        if(ex1->_type != "BOOL" || ex2->_type != "BOOL") {
            output::errorMismatch(line);
            exit(0);
        }
        _e1 = ex1;
        _e2 = ex2;
        _type = "BOOL";
        _name = "n/a";
    }
};
class Not : public STEntry{
public:
    STEntry* _e;
    Not(STEntry* e,int line){
        if(e->_type != "BOOL"){
            output::errorMismatch(line);
            exit(0);
        }
        _e = e;
        _type = "BOOL";
        _name = "NOT";
    }
};



#endif //HW3_PARSER_H
