

#include "SymbolTable.h"

SymbolTable::SymbolTable(int offset):_inWhile(false),_current_offset(offset),_retType("") {}
SymbolTable::~SymbolTable() {}

void SymbolTable::insert(STEntry *entry) {
    //if(_current_offset == 0 && entry->_isFunc) //IT's a scope of a function
    //    _retType = entry->_type;
    //cout << "OFF: " <<_current_offset;
    entry->_offset = _current_offset;
    _current_offset++;
    _symbols.push_back(entry);
}

void SymbolTable::pop() {
    _symbols.pop_back();
}

void SymbolTable::updateOffsetsRange(int range){
    for(int i = 0; i < range; i++){
        _symbols[i]->_offset = (-1) * (i+1) ;
    }
    _current_offset = 0;
}

bool SymbolTable::contains(string name){
    for(auto i = _symbols.begin() ; i != _symbols.end(); i++){
        if((*i)->_name == name){
            return true;
        }else{
            if((*i)->_isEnumType){
                if((dynamic_cast<EnumType*>(*i))->containsEnumerator(name))
                    return true;
            }
        }
    }
    return false;
}

bool SymbolTable::containsVar(string name) {
    for (auto i = _symbols.begin(); i != _symbols.end(); i++) {
        if ((*i)->_name == name) {
            if ((*i)->_isEnumType == false && (*i)->_isFunc == false)
                return true;
        }else{
            if((*i)->_isEnumType){
                if((dynamic_cast<EnumType*>(*i))->containsEnumerator(name))
                    return true;
            }
        }
    }
    return false;
}

STEntry* SymbolTable::getEntryByName(string name){
    for(auto i = _symbols.begin() ; i != _symbols.end(); i++){
        if((*i)->_name == name) return *i;
        if((*i)->_isEnumType){
            if ((dynamic_cast<EnumType*>(*i))->containsEnumerator(name)){
                return new STEntry(name,"enum "+(*i)->_name);
            }
        }

    }
    return nullptr;
}

bool SymbolTable::hasMain() {
    Function* candidate;
    if(_retType != "") return false; //We are inside a function. Main cannot be declared here.

    for(auto i = _symbols.begin() ; i != _symbols.end(); i++){
        if((*i)->_name == "main"){
            candidate = dynamic_cast<Function*>(*i);
            if(candidate->_argTypes.size() == 0 && candidate->_type == "VOID")
                return true;
        }
    }
    return false;
}

void SymbolTable::verifyRetType(string type,int line) {
    if(_retType == "") {
        cout << "Scope isn't function!!";
        SymbolTable::print();
    }
    if(_retType != type){
        if(_retType == "INT" && type == "BYTE")
        { /* Do nothing, it's conversion. */ }
        else
        {
            output::errorMismatch(line);
            exit(0);
        }
    }
}

void SymbolTable::print(){

		//cout << "----- begin regular table -----\n";
        for(int i = 0; i < _symbols.size(); i++){
            if(_symbols[i]->_isFunc){
                Function* f = dynamic_cast<Function*>(_symbols[i]);
                output::printID(f->_name,0,f->_funcType);
            }
            else if(_symbols[i]->_isEnumType)
            {     /*Dont print enums at first */       }
            else
                output::printID(_symbols[i]->_name,_symbols[i]->_offset,_symbols[i]->_type);
        }
		//cout << "----- end table -----\n";

        //cout << "----- begin enum table -----\n";
        for(int i = 0; i < _symbols.size(); i++){
            if(_symbols[i]->_isEnumType){
                EnumType* e = dynamic_cast<EnumType*>(_symbols[i]);
                output::printEnumType(e->_name,e->_enumerators);
            }
        }
        //cout << "----- end table -----\n";
	
}
