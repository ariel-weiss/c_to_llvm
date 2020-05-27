

#ifndef HW3_SYMBOLTABLE_H
#define HW3_SYMBOLTABLE_H

#include "parser.h"
#include <stack>
#include <iostream>
#include <string>
using namespace std;
enum ID_TYPE {Var, Enum, Id};

class SymbolTable {
public:
    SymbolTable(int offset=0);
    ~SymbolTable();

    vector<STEntry*> _symbols;
    bool _inWhile;
    int _current_offset;
    string _retType;

    void insert(STEntry* entry);
    void pop();
    void updateOffsetsRange(int range);

    bool contains(string name);
    bool containsVar(string name);

    STEntry* getEntryByName(string name);
    bool hasMain();
    void verifyRetType(string type,int line);
	void print();

    /* Assuming only one function in a table, return that function. */
	Function* getFunctionSymbol();

};
class TablesStack{
public:

    vector<SymbolTable*> _tables;
    TablesStack() = default;
    ~TablesStack() = default;

    void insertToTop(STEntry* x){
        _tables.back()->insert(x);
    }
    void popFromTop(){
        _tables.back()->pop();
    }
    void insertToSecond(STEntry* x){
        auto i = _tables.end();
        i--;//last element
        i--;//second last
        (*i)->insert(x);
		//(*i)->print();
    }

    void push(SymbolTable* st){
        _tables.push_back(st);
    }

    void pushNew(){
        int curr = _tables.back()->_current_offset;
        SymbolTable *st = new SymbolTable(curr);
        if(_tables.back()->_inWhile) st->_inWhile = true;
        push(st);
    }

    void pushNewWithOffset(int offset=-1){
        int ofst = _tables.back()->_current_offset; //Check for null? No need, global scope exists.
        if(offset != -1) ofst = offset;
        SymbolTable *st = new SymbolTable(ofst);
        if(_tables.back()->_inWhile) st->_inWhile = true;
        push(st);
    }

    SymbolTable* top(){
        return _tables.back();
    }
    SymbolTable* second(){//ASSERT THERE IS A SECOND ONE!
        auto i = _tables.end();
        i--;//last element
        i--;//second last
        return *i;
    }

    void CheckForMain(){
        /* TODO: Check if a valid main is defined. */

//        bool hasMain = false;
//        for(auto i = _tables.begin() ; i != _tables.end(); i++){
//            if((*i)->hasMain()) hasMain = true;
//        }
        if(_tables[0]->hasMain()) return;
        output::errorMainMissing();
        exit(0);
//        if(!hasMain){
//
//        }
    }

    bool contains(string name){

		for(auto i = _tables.begin() ; i != _tables.end(); i++){
			//(*i)->print(); //For DEBUG
            if ((*i)->contains(name))
                    return true;
        }
        return false;
    }

    STEntry* findInTables(string name){
        STEntry* result = nullptr;
        for(auto i = _tables.begin() ; i != _tables.end(); i++){
            result = (*i)->getEntryByName(name);
            if( result != nullptr) return result;
        }
        return nullptr;
    }

    bool isVarExist(string name) {
        return contains(name) ;
    }

    void verifyVarNotDefined(string name,int line){
        if(contains(name)) {
            output::errorDef(line, name);
            exit(0);
        }
    }

    void verifyVarYesDefined(string name , int line){
        if(!contains(name)) {
            output::errorUndef(line, name);
            exit(0);
        }
    }

    void verifyItIsVar(string name , int line){
        bool res=false;
        for(auto i = _tables.begin() ; i != _tables.end(); i++){
            //(*i)->print(); //For DEBUG
            if ((*i)->containsVar(name)) {
                res = true;
                break;
            }
        }
        if(!res) {
            output::errorUndef(line, name);
            exit(0);
        }
    }

    void verifyVarDef(string name,int line,bool isFunc=false){
        if(name == "n/a" || isFunc ) /* Its not a Var */
            return;

        bool res=false;
        for(auto i = _tables.begin() ; i != _tables.end(); i++){
            //(*i)->print(); //For DEBUG
            if ((*i)->containsVar(name)) {
                res = true;
                break;
            }
        }
        if(!res) {
            output::errorUndef(line, name);
            exit(0);
        }
    }

    void checkVarsTypes(string type1, string type2,int line,string name="") {

        if(type1 == "ENUM_TYPE" && type2.substr(0,4) == "enum")
            type2 = "ENUM_TYPE";

        if(type1 != type2){
            if(type1 == "INT" && type2 =="BYTE")
            { /* It's OK, convert. */ }
            else if(type1.substr(type1.find(' '),type1.length()) == type2.substr(type2.find(' '),type2.length()))
            { /* Same enum type, it's ok. */}
            else if(type1.substr(0,4) == "enum"){
                output::errorUndefEnumValue(line,name);
                exit(0);
            }
            else{
                output::errorMismatch(line);
                exit(0);
            }
        }
    }

    void checkVarsImplicitTypes(string name1, string type2,int line) {
        STEntry* ste = findInTables(name1);
        if (ste == nullptr) {
            output::errorUndef(line,name1);
            exit(0);
        }
        string type1 = ste->_type;
        checkVarsTypes(type1,type2,line,name1);
    }

    void checkLastReturn(string type,int line){
        if(type == "VOID")
            type = "TYPE_MISMATCH";
        if(type == "EMPTY")
            type = "VOID";
        auto i = _tables.begin();
        i++;//global scope
        //i++;//second scope

        (*i)->verifyRetType(type,line);
    }

    /* :::::: While Functions :::::: */
    void enterWhile(){
        _tables.back()->_inWhile=true;
    }
    void exitWhile(){
        _tables.back()->_inWhile=false;
    }
    void checkBreak(int line){
        if(!_tables.back()->_inWhile){
            output::errorUnexpectedBreak(line);
            exit(0);
        }
    }
    void checkContinue(int line){
        if(!_tables.back()->_inWhile){
            output::errorUnexpectedContinue(line);
            exit(0);
        }
    }
    /* :::::::::::::::::::::::::::: */
    Function* getFunctionFromCall(string name,int line){
        if(!isVarExist(name)){
            output::errorUndefFunc(line, name);
            exit(0);
        }
        STEntry* temp = findInTables(name);
        if(temp == nullptr || !(temp->_isFunc)){
            output::errorUndefFunc(line,name);
            exit(0);
        }
        Function* func = dynamic_cast<Function*>(temp);
        return func;
    }

    EnumType* getEnumTypeFromTable(string name,int line){
        //verifyEnumTypeDef(name,line);
        STEntry* temp = findInTables(name);
        if(temp == nullptr || !(temp->_isEnumType)){

            output::errorUndefEnum(line,name);
            exit(0);
        }
        EnumType* e = dynamic_cast<EnumType*>(temp);
        return e;
    }

    void printCurrentTabel(){
        _tables.back()->print();
    }
    void pop(){
        _tables.pop_back();
    }

    void verifyEnumTypeDef(string name,int line){
        //if (!contains(name,Enum)){
        //    output::errorUndefEnum(line, name);
        //    exit(0);
        //}
        getEnumTypeFromTable(name,line);
    }

    bool checkEnumeratorAssign(EnumType* e,STEntry* ste,string x,int line){


        for(string en : e->_enumerators){

            if(en == ste->_name)
                return true; // It's valid enumerator.
        }

        for(string en : e->_enumerators){

            if(en == ste->_enumVal)
                return false;// It's a variable.
        }
        //Not found:

        output::errorUndefEnumValue(line, x);
        exit(0);
    }

    /* ================================================================== */
    //Code for ex5:
    Function* getMeMyFunction(){
        Function* res;
        //Get me the first instance of Function that you encounter
        for(auto table : _tables){
            res = table->getFunctionSymbol();

            if(res != nullptr) return res;
        }
//        auto i = _tables.end();
//        i--;
//        for(; i != _tables.begin(); i--){
//
//            res = (*i)->getFunctionSymbol();
//            if(res != nullptr) return res;
//        }
//        res = (*i)->getFunctionSymbol();
//        if(res != nullptr) return res;
        //Assert: unreachable!
        return nullptr;
    }
};


#endif //HW3_SYMBOLTABLE_H
