/*
 * Generates K arrow PSVN files when state variables are state-of-an-arrow
 */
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
using namespace std;
#define PEG_NUM 3
int D;
string generate_hanoi_rule(int src, int des, int d);

string convertInt(int number)
{
   ostringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
      cout << "provide D as an argument please." <<endl;
      return -1;
    }

    D=atoi(argv[1]);
    string havij="";

//    string domain="BIT";
//	havij = havij+"# list of domains\n";
//	havij = havij+"Domain "+domain+" 2 #0,1\n";
//	havij = havij+ "\t";
//	for (int i=0;i<2;i++)
//		havij= havij+convertInt(i)+" ";
	havij = havij+"# number of state variables (positions in the state vector)\n";
	havij=havij+convertInt(D*PEG_NUM)+"\n";
	havij=havij+"# domain for each variable\n"+
				"# an integer I is the numbers 0 to I-1\n"+
				"# an integer I followed by N is the numbers 1 to I\n";
	for (int i=0;i<D*PEG_NUM;i++)
		havij=havij+"2 ";
//		havij=havij+domain+" ";


//    havij =     "# number of state variables (positions in the state vector)\n";
//    havij=havij+convertInt(D*PEG_NUM)+"\n";
//    havij=havij+"# domain for each variable\n"+
//                "# an integer I is the numbers 0 to I-1\n"+
//                "# an integer I followed by N is the numbers 1 to I\n";
//    for (int i=0;i<D*PEG_NUM;i++)
//        havij=havij+"2 ";
//

    havij=havij+"\n";
    havij=havij+"# rules <condition> => <action>\n"+
                "# where both <condition> and <action> are NUMBER_VARIABLES entries wide\n";
    for (int d=0; d<D; d++)
    {
        for (int i=0; i<PEG_NUM;i++)
        {
            for(int j=0;j<PEG_NUM;j++)
            {
                if (i==j)
                    continue;
                havij=havij+generate_hanoi_rule(i,j,d);//source_col, dest_col, disk
            }
        }
    }
    havij=havij+"# goal rule <condition>\n"+
                "# where condition is like a normal rule condition\n"+
                "# multiple goal conditions are logically disjunctive\n"+
                "GOAL ";
    for (int i=0;i<PEG_NUM*D;i++)
    {
        if(i < (PEG_NUM-1)*D)
            havij=havij+ "0 ";
        else
            havij=havij+ "1 ";
    }
    havij=havij+"\n";
//    printf("%s",havij);
    cout << havij;


}


string generate_hanoi_rule(int src, int des, int d)
{
    string tstr1="";
    string tstr2="";
    int rev_flag= 0;
    if (src < des)
    {
        rev_flag= 1;
        int tmp= src;
        src= des;
        des=tmp;
    }
    for (int i=0; i< PEG_NUM*D; i++)
    {
        tstr1.append("- ");
        tstr2.append("- ");
    }
    for (int i=0; i<d; i++)
    {
        tstr1[2*(src*D+i)]='0';
        tstr1[2*(src*D+i)+1]=' ';
        tstr2[2*(src*D+i)]='0';
        tstr2[2*(src*D+i)+1]=' ';
        tstr1[2*(des*D+i)]='0';
        tstr1[2*(des*D+i)+1]=' ';
        tstr2[2*(des*D+i)]='0';
        tstr2[2*(des*D+i)+1]=' ';
    }
    if(!rev_flag)
    {
        tstr1.replace(2*(src*D+d),2, "1 ");
        tstr1.replace(2*(des*D+d),2, "0 ");
        tstr2.replace(2*(src*D+d),2, "0 ");
        tstr2.replace(2*(des*D+d),2, "1 ");
    }
    else
    {
        tstr1.replace(2*(src*D+d),2, "0 ");
        tstr1.replace(2*(des*D+d),2, "1 ");
        tstr2.replace(2*(src*D+d),2, "1 ");
        tstr2.replace(2*(des*D+d),2, "0 ");
    }
    tstr1=tstr1+"=> "+tstr2+"\n";
    return tstr1;
}
