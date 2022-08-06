#ifndef HDF5JULIA_H
#define HDF5JULIA_H

#endif // HDF5JULIA_H


#include <iostream>
#include <string>
#include <julia.h>
#include <stdexcept>
#include <vector>

class NumericalString
{

private:
    long int number{0};
    std::string strNum;

    void buildString(){
        strNum = std::to_string(number);
    }
public:
    NumericalString(long int n) {
        number = n;
        buildString();
    }

    void update(long int newNumber){
        number = newNumber;
        buildString();
    }
    void operator++(){
        ++number;
        buildString();
    }

    void operator--(){
        --number;
        buildString();
    }

    long int getInt(){
        return number;
    }

    std::string getStr(){
        return strNum;
    }

    std::string getIndexStr(){
        return "[" + strNum + "]";
    }

};

class JuliaCommand
{
    private:
        std::string com;

    public:
        JuliaCommand(const std::string s) {com = s;}

        std::string get_str(){
            return com;
        }

        void set_command(std::string newCommand){
            com = newCommand;
        }

        void ConcatToRight(std::string s){
            com = com + s;
        }

        void ConcatToLeft(std::string s){
            com = s + com;
        }

        JuliaCommand operator+(JuliaCommand otherCom){
            JuliaCommand newCommand{com + otherCom.get_str()};
            return newCommand;
        }

        JuliaCommand operator+(std::string s){
            JuliaCommand newCommand{com + s};
            return newCommand;
        }


        jl_value_t *evaluate(){
            std::cout
                    << "Evaluating Julia command: \""
                    << com
                    << "\" ... \n";

            auto res = jl_eval_string(com.c_str());
            std::cout << "Result: ";

            if (jl_exception_occurred()){

                std::cout
                        << "Faild! Because of \""
                        << jl_typeof_str(jl_exception_occurred())
                        << "\" from Julia "
                        << "\n\n";

                throw std::runtime_error("Julia Command Evaluations Faild");

            } else {
                std::cout << "Done!\n\n";
            }

            return res;
        }

};


class JuliaStatus
{
    public:
        JuliaStatus(jl_value_t *output) {
            value = output;
        }

        jl_value_t * getValue() {
            return value;
        }


    private:
        jl_value_t *value;
};

class JuliaValue : public JuliaStatus
{
    public:
        JuliaValue(std::string varName, bool ThisIsArray = false): JuliaStatus (jl_eval_string(varName.c_str())) {
            if (varName.find(" ") == std::string::npos)
                throw std::invalid_argument("A variable name does not consist white space.");

            if (jl_exception_occurred())
                throw std::invalid_argument("This variable does not exist.");


            datatype = jl_typeof_str(getValue());
            arrayFlag = ThisIsArray;
        }

        std::string get_name(){
            return name;
        }

        void set_name(std::string new_name, bool ThisIsArray = false){
            if (new_name.find(" ") != std::string::npos)
                throw std::invalid_argument("A variable name does not consist white space.");

            if (jl_exception_occurred())
                throw std::invalid_argument("This variable does not exist.");


            datatype = jl_typeof_str(getValue());
            arrayFlag = ThisIsArray;
        }

    private:
        std::string name;
        std::string datatype;
        bool arrayFlag{};


};



class AbstractRow
{
    public:
        AbstractRow(int index, std::string parentDataSet) {
            parent.set_name(parentDataSet);
            rowIndex.update(index);
        }

        NumericalString whatIsTheIndex(){
            return rowIndex;
        }

        JuliaValue whatIsTheParent(){
            return parent;
        }

        size_t length(){
            JuliaCommand getlength{"length(" + parent.get_name() + "[" + rowIndex.getStr() + "])"};
            return static_cast<size_t>(jl_unbox_int64(getlength.evaluate()));
        }

    private:
        NumericalString rowIndex{0};
        JuliaValue parent{""};
};

class Data
{
    public:
        void setter(jl_value_t* value) {
            len = jl_array_len(value); DataPointer = (uint16_t *)jl_array_data((jl_value_t *)value);

        }

        size_t lenght(){
            return len;
        }

        uint16_t operator[](size_t i) {
            if (i < lenght())
                return DataPointer[i];
            throw std::runtime_error("Index Out of Range!");
        }

    private:
        uint16_t* DataPointer;
        size_t len;
};

class Row: AbstractRow
{
    public:
        Row(size_t index, std::string parentDataSet): AbstractRow(index, parentDataSet) {
                INDEX.update(index);
                Accessing.ConcatToRight(parentDataSet + INDEX.getIndexStr());



                lsnum         = jl_unbox_uint32((Accessing + ".lsnum").evaluate());
                nbnum         = jl_unbox_uint32((Accessing + ".nbnum").evaluate());
//                runnum        = jl_unbox_uint32((Accessing + ".runnum").evaluate());
//                fillnum       = jl_unbox_uint32((Accessing + ".fillnum").evaluate());
//                timestampsec  = jl_unbox_uint32((Accessing + ".timestampsec").evaluate());
//                timestampmsec = jl_unbox_uint32((Accessing + ".timestampmsec").evaluate());


//                publishnnb    = jl_unbox_uint8((Accessing + ".publishnnb").evaluate());
//                datasourceid  = jl_unbox_uint8((Accessing + ".datasourceid").evaluate());
//                algoid        = jl_unbox_uint8((Accessing + ".algoid").evaluate());
                channelid     = jl_unbox_uint8((Accessing + ".channelid").evaluate());

                data.setter((Accessing + ".data").evaluate());

            }
        size_t get_index() {
            return static_cast<size_t>(INDEX.getInt());
        }


            uint32_t lsnum, nbnum;
            uint8_t channelid;
            Data data;

////        Other Values:

//            uint32_t runnum, fillnum, timestampsec, timestampmsec;
//            uint8_t publishnnb, datasourceid, algoid;


        private:
            NumericalString INDEX{0};
            JuliaCommand Accessing{""};

};

class HDF5File
{
    public:
        HDF5File(std::string FileName, std::string vname, std::string Path = "", std::string Mode = "r") {
            Varname = "HDF5_" + vname;
            name = FileName;
            pathof = Path;
            mode = Mode;
            dir = pathof + FileName;

            openFile();

            std::cout << "Reading Rows: " << std::endl;
            const auto total = length();
            for (size_t index = 1; index <= total; ++index) {
                r.push_back(Row(index, "bcm1fagghist"));
            }
        }


        void closeFile(){
            JuliaCommand c{"close(" + Varname + ")"};
            c.evaluate();
        }

        size_t length(){
            JuliaCommand getlength{"length(" + Varname + "[\"bcm1fagghist\"])"};
            return static_cast<size_t>(jl_unbox_int64(getlength.evaluate()));
        }

        std::vector<Row> rows(){return r;}

    private:
        std::string Varname;
        std::string name;
        std::string pathof;
        std::string dir;
        std::string mode;
        std::vector<Row> r;

        JuliaStatus getIndex(const std::string assignVar, const std::string Varname, const int index){
            NumericalString i{index};
            JuliaCommand c{assignVar + " = " + Varname + "[" + i.getStr() + "]"};
            return JuliaStatus(c.evaluate());
        }

        JuliaStatus getIndex(const std::string assignVar, const std::string Varname, const std::string index){
            JuliaCommand c{assignVar + " = " + Varname + "[\"" + index + "\"]"};
            return JuliaStatus(c.evaluate());
        }

        void openFile(){
            JuliaCommand c1{Varname + " = " + "HDF5.h5open(\"" + dir + "\", " + "\"" + mode + "\")"};
            c1.evaluate();
            JuliaCommand c2{"bcm1fagghist = " + Varname + "[\"bcm1fagghist\"]" };
            c2.evaluate();

        }
};



void using_Pkg(){
    jl_eval_string("print(\"Trying to load Pkg...\n\n\"); using Pkg; print(\"Successfully added!\n\n\")");
    return ;
}

void using_modules(){
    std::cout << "Initializing... trying to use: HDF5, H5Zblosc, H5Zbzip2, H5Zlz4, H5Zzstd" << std::endl;
    jl_eval_string("using HDF5, H5Zblosc, H5Zbzip2, H5Zlz4, H5Zzstd; print(\"Done!\nHDF5, H5Zblosc, H5Zbzip2, H5Zlz4, H5Zzstd are being used.\n\n\")");
    return ;
}

void install_dependacy(){
        using_Pkg();
        std::cout << "Initializing... Dependecies: HDF5, H5Zblosc, H5Zbzip2, H5Zlz4, H5Zzstd\n" << std::endl;

        std::cout << "Installing HDF5..." << std::endl;
        jl_eval_string("Pkg.add(\"HDF5\"); print(\"Done!\n\"");

        std::cout << "Installing H5Zblosc..." << std::endl;
        jl_eval_string("Pkg.add(\"H5Zblosc\"); print(\"Done!\n\"");

        std::cout << "Installing H5Zbzip2..." << std::endl;
        jl_eval_string("Pkg.add(\"H5Zbzip2\"); print(\"Done!\n\"");

        std::cout << "Installing H5Zlz4..." << std::endl;
        jl_eval_string("Pkg.add(\"H5Zlz4\"); print(\"Done!\n\"");

        std::cout << "Installing H5Zzstd..." << std::endl;
        jl_eval_string("Pkg.add(\"H5Zzstd\"); print(\"Done!\n\"");

        return ;
}
