
#ifndef HW3_REGISTERSGEN_H
#define HW3_REGISTERSGEN_H
#include <iostream>
#include <string>
#include <iomanip>
#include "bp.hpp"
#include "parser.h"

#define BUFFER (CodeBuffer::instance())
using std::string;

class CodeGen{
    public:
    int counter;

    CodeGen():counter(0){};
    ~CodeGen() = default;

    int genFresh(){
      return counter++;
    };
//    int getCurrent(){
//        return counter;
//    };

    string genFreshVar(){
        string str = "%var"+std::to_string(genFresh());
        return str;
    };
    string genFreshString(){
        string str = "@.str_"+std::to_string(genFresh());
        return str;
    };
//    string getCurrentVar(){
//        string str = "%var"+std::to_string(getCurrent());
//        return str;
//    };

    string genGetElement(string a_name, string a_type,string a_size,string elem){
        string res = "getelementptr [" + a_size +"x"+ a_type +"] [" + a_size +"x"+ a_type +"]* "+a_name+", "+a_type+" 0, "+a_type+" "+elem;
        return res;
    }

    void printList(vector<pair<int,BranchLabelIndex>> lst){
        for(auto item : lst){
            cout<<"Item: "<<item.first<<"\n";
        }
    }
    int emitCommand(const string &command, const string &comment) {
//        stringstream ss;
//        auto actualComment = comment.empty() ? "" : "; " + comment;
//        if (!command.empty()) {
//            ss << std::left << "   " <<std::setw(80) <<command;
//        }
//        ss << actualComment;
        return CodeBuffer::instance().emit(command);
    }
    /*******************************************************************/
    /***=============================================================***/
    /***=============================================================***/
    /*******************************************************************/
    void initVar(string var,int offset,int argsNum=0){
        string num = std::to_string(argsNum);
        if(offset >= 0)
            emitCommand(var+" = getelementptr [50 x i32], [50 x i32]* %fp, i32 0 , i32 "+std::to_string(offset),"load local");
        else
            emitCommand(var+" = getelementptr ["+num+" x i32], ["+num+" x i32]* %args, i32 0 , i32 "+std::to_string(((offset+1)*(-1))),"load arg");
    }
    string eval_exp(STEntry* exp){
        string tempv_result = genFreshVar();
        /* True Area */
        int redundent_addt = BUFFER.emit("br label @"); //For compilation only..
            string truelbl = BUFFER.genLabel();
            BUFFER.bpatch(BUFFER.makelist({redundent_addt,FIRST}),truelbl);
            int true_addt = emitCommand("br label @", "jump to Next label");
            BUFFER.bpatch(exp->_truelist, truelbl);
        /* False Area */
            string falselbl = BUFFER.genLabel();
            int false_addt = emitCommand("br label @", "jump to Next label");
            BUFFER.bpatch(exp->_falselist, falselbl);
        /* Evaluate with phi! */
        string nextlbl = BUFFER.genLabel();
            BUFFER.bpatch(BUFFER.makelist({true_addt,FIRST}),nextlbl);
            BUFFER.bpatch(BUFFER.makelist({false_addt,FIRST}),nextlbl);
            emitCommand(tempv_result+" = phi i32 [ 1, %"+truelbl+" ] , [ 0, %"+falselbl+"]","evaluate");
        return tempv_result;
    }
    /*******************************************************************/
    /***                         PRE PROG                       ********/
    /*******************************************************************/
    void pre_prog_rule(){

        BUFFER.emitGlobal("declare i32 @printf(i8*, ...)");
        BUFFER.emitGlobal("declare void @exit(i32)");
        BUFFER.emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
        BUFFER.emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
        BUFFER.emitGlobal("@.zero_div_err = constant [24 x i8] c\"Error division by zero\\0A\\00\"");
        BUFFER.emit("define void @printi(i32) {");
        BUFFER.emit("call i32 (i8*, ...) @printf(i8* getelementptr([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0)");
        BUFFER.emit("ret void");
        BUFFER.emit("}");
        BUFFER.emit("define void @print(i8*) {");
        BUFFER.emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0), i8* %0)");
        BUFFER.emit("ret void");
        BUFFER.emit("}");
    }
    /*******************************************************************/
    /***      S =                Call F()                       ********/
    /*******************************************************************/
    void call(STEntry* $$,Function* func,Explist* explst){
        //First check if it's the print function
        if(func->_name == "print"){
            STEntry* str = explst->_exps[0];
            //It's a STRING entry.
            string len = std::to_string(str->_name.length()+1);
            BUFFER.emit("call void @print( i8* getelementptr (["+len+" x i8], ["+len+" x i8]* "+str->_place+", i32 0, i32 0) )");
            $$->_place =  "void";
            return;
        }
        //Continue
        string retFormat = func->_type;
        if(retFormat == "VOID") retFormat = "void";
        else retFormat = "i32";
        /***********************/
        /* Evaluate Arguments :*/
        string argsFormat = "";
        string exceptedArgsFormat = "";
            for(auto it = explst->_exps.rbegin();it != explst->_exps.rend();++it){
                string tempplace = (*it)->_place;
//                if((*it)->_type == "BOOL"){
//                    tempplace=eval_exp((*it));
//                }
                argsFormat = argsFormat + "i32 "+tempplace+",";
                exceptedArgsFormat = exceptedArgsFormat + "i32 ,";
            }
        /***********************/
        argsFormat = argsFormat.substr(0,argsFormat.length()-1);
        exceptedArgsFormat = exceptedArgsFormat.substr(0,exceptedArgsFormat.length()-1);
        string res = "call "+retFormat+"("+exceptedArgsFormat+")"+" @"+func->_name+"("+argsFormat+")";
        if(retFormat == "void") {
            emitCommand(res, "CALL");
            $$->_place = "void";
        }
        else{
            string var = genFreshVar();
            emitCommand(var+" = "+res,"CALL");
            $$->_place = var;
            if(func->_type == "BOOL"){
                /* If bool, evaluate please. */
                string tempv = genFreshVar();
                /* Evaluate with phi here? */
                emitCommand(tempv+" = icmp ne i32 0, "+var,"Evaluate ["+func->_name+"]");
                int addt = emitCommand("br i1 "+tempv+", label @ , label @","");
                $$->_truelist = BUFFER.makelist({addt,FIRST});
                $$->_falselist = BUFFER.makelist({addt,SECOND});
                //In case of end-statement:
                $$->_nextlist = BUFFER.merge((BUFFER.makelist({addt, FIRST})),
                                             (BUFFER.makelist({addt, SECOND})));
            }
        }
    }
    void call_noargs(STEntry* $$, Function* func){
        string retFormat = func->_type;
        if(retFormat == "VOID") retFormat = "void";
        else retFormat = "i32";
        string res = "call "+retFormat+" @"+func->_name+"()";

        if(retFormat == "void") {
            emitCommand(res, "CALL");
            $$->_place =  "void";
        }
        else{
            string var = genFreshVar();
            emitCommand(var+" = "+res,"CALL");
            $$->_place = var;

            if(func->_type == "BOOL"){
                /* If bool, evaluate please. */
                string tempv = genFreshVar();
                /* Evaluate with phi here */
                emitCommand(tempv+" = icmp ne i32 0, "+var,"Evaluate ["+func->_name+"]");
                int addt = emitCommand("br i1 "+tempv+", label @ , label @","");
                $$->_truelist = BUFFER.makelist({addt,FIRST});
                $$->_falselist = BUFFER.makelist({addt,SECOND});

                $$->_nextlist = BUFFER.merge((BUFFER.makelist({addt, FIRST})),
                                             (BUFFER.makelist({addt, SECOND})));
            }
        }

    }
    /*******************************************************************/
    /***      Exp =           Exp BINOP Exp                     ********/
    /*******************************************************************/
    void binop_rule(STEntry* $$,STEntry* $1,STEntry* $2,STEntry* $3) {
        int addt;
        string temp = genFreshVar();
        string zero_div_t = genFreshVar();
        string div_err_lbl;
        string no_div_err_lbl;
        /* TODO: Handle BYTE overflow: */
        string op_type = "i32";
        string lhs = $1->_place;
        string rhs = $3->_place;
        string res = temp;
        if($2->_name != "/" && $1->_type == "BYTE" && $3->_type == "BYTE"){
            lhs = genFreshVar();
            emitCommand(lhs+" = trunc i32 "+$1->_place+" to i8","Handle byte overflow, lhs");
            rhs = genFreshVar();
            emitCommand(rhs+" = trunc i32 "+$3->_place+" to i8","Handle byte overflow, rhs");
            op_type = "i8";
        }

        if($2->_name == "+") emitCommand(temp+" = add "+op_type+" "+lhs+", "+rhs,$1->_name + " [+] " + $3->_name);
        if($2->_name == "-") emitCommand(temp+" = sub "+op_type+" "+lhs+", "+rhs,$1->_name + " [-] " + $3->_name);
        if($2->_name == "*") emitCommand(temp+" = mul "+op_type+" "+lhs+", "+rhs,$1->_name + " [*] " + $3->_name);
        if($2->_name == "/") {
            /* TODO: Add zero div error check */
            emitCommand(zero_div_t+" = icmp eq i32 "+$3->_place+", 0","Check for zero div");
            addt = emitCommand("br i1 "+zero_div_t+" , label @ , label @","");
            div_err_lbl = BUFFER.genLabel();
            emitCommand("call i32 (i8*, ...) @printf(i8* getelementptr([24 x i8], [24 x i8]* @.zero_div_err, i32 0, i32 0) )","Printf()");
            emitCommand("call void @exit(i32 0)","Exit()");
            int redundent_jump = emitCommand("br label @","...");
            no_div_err_lbl = BUFFER.genLabel();
            emitCommand(temp+" = sdiv "+op_type+" "+lhs+", "+rhs,$1->_name + " [/] " + $3->_name);
            BUFFER.bpatch(BUFFER.makelist({addt,SECOND}),no_div_err_lbl);
            BUFFER.bpatch(BUFFER.makelist({addt,FIRST}),div_err_lbl);
            BUFFER.bpatch(BUFFER.makelist({redundent_jump,FIRST}),no_div_err_lbl);
        }
        /* TODO: Check overflow, Check un/signed */
        if(op_type == "i8"){
            string res = genFreshVar();
            emitCommand(res+" = zext i8 "+temp+" to i32","Handle byte overflow, result");
            temp = res;
        }
        $$->_place = temp;
        //emitCommand("store i32 "+temp+" , i32* %temp","Store the result in temp local");
    }
    /*******************************************************************/
    /***      Exp =           Exp RELOP Exp                     ********/
    /*******************************************************************/
    void relop_rule(STEntry* $$,STEntry* $1,STEntry* $2,STEntry* $3){
        int addt;
        string temp = genFreshVar();
        /* Greating Label */
//        int addtM = emitCommand("br label @"," Label ["+$2->_name+"] Exp");
//        string lbl = BUFFER.genLabel();
//        BUFFER.bpatch(BUFFER.makelist({addtM,FIRST}),lbl);
        /* Do Operation and Condition */
        if($2->_name == ">") emitCommand(temp+" = icmp sgt i32 "+$1->_place+", "+$3->_place,$1->_name + " [>] " + $3->_name);
        if($2->_name == ">=") emitCommand(temp+" = icmp sge i32 "+$1->_place+", "+$3->_place,$1->_name + " [>=] " + $3->_name);
        if($2->_name == "<") emitCommand(temp+" = icmp slt i32 "+$1->_place+", "+$3->_place,$1->_name + " [<] " + $3->_name);
        if($2->_name == "<=") emitCommand(temp+" = icmp sle i32 "+$1->_place+", "+$3->_place,$1->_name + " [<=] " + $3->_name);
        if($2->_name == "==") emitCommand(temp+" = icmp eq i32 "+$1->_place+", "+$3->_place,$1->_name + " [==] " + $3->_name);
        if($2->_name == "!=") emitCommand(temp+" = icmp ne i32 "+$1->_place+", "+$3->_place,$1->_name + " [!=] " + $3->_name);
        addt = emitCommand("br i1 "+temp+", label @ , label @","");
        $$->_truelist = BUFFER.makelist({addt,FIRST});
        $$->_falselist = BUFFER.makelist({addt,SECOND});
        //$$->_nextlist = BUFFER.merge($$->_truelist,$$->_falselist);
        $$->_place = temp;
        //$$->_lbl = lbl;
        //emitCommand("store i32 "+temp+" , i32* %temp","Store the result in Temp");
    }
    /*******************************************************************/
    /***      Exp =           Exp AND M Exp                     ********/
    /*******************************************************************/
    void and_rule(STEntry* $$,STEntry* $1,STEntry* $2,STEntry* $3,STEntry* $4){
        BUFFER.bpatch($1->_truelist,$3->_name);
        $$->_truelist = $4->_truelist;
        $$->_falselist = BUFFER.merge($1->_falselist, $4->_falselist);

        //$$->_lbl = lbl;
        $$->_nextlist = $$->_falselist;//Not logical
        //BUFFER.bpatch($1->_truelist,$3->_name);
        //BUFFER.emit("STORE IN TEMP");
    }
    /*******************************************************************/
    /***      Exp =           Exp OR M Exp                     ********/
    /*******************************************************************/
    void or_rule(STEntry* $$,STEntry* $1,STEntry* $2,STEntry* $3,STEntry* $4){
        BUFFER.bpatch($1->_falselist,$3->_name);
        $$->_falselist = $4->_falselist;
        $$->_truelist = BUFFER.merge($1->_truelist, $4->_truelist);
        //
        /* Greating Label */
//        int addtM = emitCommand("br label @"," Label [OR] Exp");
//        string lbl = BUFFER.genLabel();
//        BUFFER.bpatch(BUFFER.makelist({addtM,FIRST}),lbl);
        //string resvar = genFreshVar();
        //BUFFER.emit(resvar+" = phi i1 [ true, "+$1->_lbl+" ], [ "+$4->_place+", "+$4->_lbl+" ]");
        //$$->_lbl = lbl;
    }
    /*******************************************************************/
    /***        S =          Type ID = Exp                      ********/
    /*******************************************************************/
    void type_id_asmt(STEntry* $$,STEntry* $1,STEntry* $2,STEntry* $3,STEntry* $4,STEntry* ste){

        //rgen.emitCommand(tempv1+" = load i32, i32* %temp","load from temp");

        if($4->_type != "BOOL"){
            string tempv2 = genFreshVar();
            initVar(tempv2,ste->_offset);
            emitCommand("store i32 "+$4->_place+", i32* "+tempv2,"store in fp");
            $$->_nextlist = $4->_nextlist;
            //$$->_lbl = $4->_lbl;
        }else {
            string tempv_true = genFreshVar();
            string tempv_false = genFreshVar();
            /* True Area */
            int redundent_addt = BUFFER.emit("br label @"); //For compilation only..
            string truelbl = BUFFER.genLabel();
            BUFFER.bpatch(BUFFER.makelist({redundent_addt,FIRST}),truelbl);
            initVar(tempv_true,ste->_offset);
            emitCommand("store i32 1, i32* " + tempv_true, "store TRUE in " + $2->_name);
            int true_addt = emitCommand("br label @", "jump to Next label");
            BUFFER.bpatch($4->_truelist, truelbl);
            /* False Area */
            string falselbl = BUFFER.genLabel();
            initVar(tempv_false,ste->_offset);
            emitCommand("store i32 0, i32* " + tempv_false, "store FALSE in " + $2->_name);
            int false_addt = emitCommand("br label @", "jump to Next label");
            BUFFER.bpatch($4->_falselist, falselbl);
            /* Pass the nextlist */
            $$->_nextlist = BUFFER.merge((BUFFER.makelist({true_addt, FIRST})),
                                         (BUFFER.makelist({false_addt, FIRST})));
            /* TODO : place = phi */

        }
    }
    /*******************************************************************/
    /***        S =            ID = Exp                         ********/
    /*******************************************************************/
    void id_asmt(STEntry* $$,STEntry* $1,STEntry* $2,STEntry* $3,STEntry* ste,Function* f){

        //rgen.emitCommand(tempv1+" = load i32, i32* %temp","load from temp");
        //Check local or arg..
        if($3->_type != "BOOL"){
            string tempv2 = genFreshVar();
            initVar(tempv2,ste->_offset,f->_argTypes.size());
            emitCommand("store i32 "+$3->_place+", i32* "+tempv2,"store in stack");
            $$->_nextlist = $3->_nextlist;
            //$$->_lbl = $3->_lbl;
        }else {
            string tempv_true = genFreshVar();
            string tempv_false = genFreshVar();
            /* True Area */
            int redundent_addt = BUFFER.emit("br label @"); //For compilation only..
            string truelbl = BUFFER.genLabel();
            BUFFER.bpatch(BUFFER.makelist({redundent_addt,FIRST}),truelbl);
            initVar(tempv_true,ste->_offset,f->_argTypes.size());
            emitCommand("store i32 1, i32* " + tempv_true, "store TRUE in " + $1->_name);
            int true_addt = emitCommand("br label @", "jump to Next label");
            BUFFER.bpatch($3->_truelist, truelbl);
            /* False Area */
            string falselbl = BUFFER.genLabel();
            initVar(tempv_false,ste->_offset,f->_argTypes.size());
            emitCommand("store i32 0, i32* " + tempv_false, "store FALSE in " + $2->_name);
            int false_addt = emitCommand("br label @", "jump to Next label");
            BUFFER.bpatch($3->_falselist, falselbl);
            /* Pass the nextlist */
            $$->_nextlist = BUFFER.merge((BUFFER.makelist({true_addt, FIRST})),
                                         (BUFFER.makelist({false_addt, FIRST})));
        }
    }
    /*******************************************************************/
    /***    Explist =            Exp                            ********/
    /*******************************************************************/
    void exp_eval(STEntry* $$,STEntry* $1){
        if($1->_type == "BOOL"){//Make sure..
            $1->_place = eval_exp($1);
        }
    }
    /*******************************************************************/
    /***     Exp =                ID                            ********/
    /*******************************************************************/
    void id_eval(STEntry* $$,STEntry* $1,STEntry* ste,Function* f){
        /* TODO: Load the value of ID into _place */
        /* Handle Enumerators */
        if(ste->_type.substr(0,10) == "enumerator"){
            string enumval = std::to_string(ste->_evalue);
            $$->_place = enumval;
            return;
        }
        /* Now, everything else */
        string tempvar1 = genFreshVar();
        string tempvar2 = genFreshVar();
        string offst = std::to_string(ste->_offset);
        initVar(tempvar1,ste->_offset,f->_argTypes.size());
        emitCommand(tempvar2+" = load i32, i32* "+tempvar1 ,"Load from Stack");
        $$->_place = tempvar2;
        if(ste->_type == "BOOL"){
            string tempv = genFreshVar();
            /* Evaluate with phi here */
            emitCommand(tempv+" = icmp ne i32 0, "+tempvar2,"Evaluate ["+$1->_name+"]");
            int addt = emitCommand("br i1 "+tempv+", label @ , label @","");
            $$->_truelist = BUFFER.makelist({addt,FIRST});
            $$->_falselist = BUFFER.makelist({addt,SECOND});
        }

    }
    /*******************************************************************/
    /***       S =            return Exp                        ********/
    /*******************************************************************/
    void ret_exp(STEntry* $$,STEntry* $1){
        if($1->_type == "BOOL"){

            $1->_place = eval_exp($1);
        }
    }

};



#endif //HW3_REGISTERSGEN_H
