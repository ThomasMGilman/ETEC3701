#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#include "kprintf.h"
#include "util.h"

char debugMsg[40];
int cycle1 = 0;

extern int file_open(const char* fname, int flags);
extern int file_read(int fd, void* buf, int num);
extern int file_seek(int fd, int offset, int whence);
extern int file_close(int fd);

static const char article6[] = {
    "Article. VI.\n\n"
    "All Debts contracted and Engagements entered into, before the Adoption "
    "of this Constitution, shall be as valid against the United States under "
    "this Constitution, as under the Confederation.\n\n"
    "This Constitution, and the Laws of the United States which shall be made "
    "in Pursuance thereof; and all Treaties made, or which shall be made, "
    "under the Authority of the United States, shall be the supreme Law of the "
    "Land; and the Judges in every State shall be bound thereby, any Thing in the "
    "Constitution or Laws of any State to the Contrary notwithstanding.\n\n"
    "The Senators and Representatives before mentioned, and the Members of "
    "the several State Legislatures, and all executive and judicial Officers, "
    "both of the United States and of the several States, shall be bound by Oath or "
    "Affirmation, to support this Constitution; but no religious Test shall ever "
    "be required as a Qualification to any Office or public Trust under the United States." };
    
static const char article2[] = {
    "Article. II.\n\nSection. 1.\n\nThe executive Power"
    " shall be vested in a President of the United Stat"
    "es of America. He shall hold his Office during the"
    " Term of four Years, and, together with the Vice P"
    "resident, chosen for the same Term, be elected, as"
    " follows:\n\nEach State shall appoint, in such Man"
    "ner as the Legislature thereof may direct, a Numbe"
    "r of Electors, equal to the whole Number of Senato"
    "rs and Representatives to which the State may be e"
    "ntitled in the Congress: but no Senator or Represe"
    "ntative, or Person holding an Office of Trust or P"
    "rofit under the United States, shall be appointed "
    "an Elector.\n\nThe Electors shall meet in their re"
    "spective States, and vote by Ballot for two Person"
    "s, of whom one at least shall not be an Inhabitant"
    " of the same State with themselves. And they shall"
    " make a List of all the Persons voted for, and of "
    "the Number of Votes for each; which List they shal"
    "l sign and certify, and transmit sealed to the Sea"
    "t of the Government of the United States, directed"
    " to the President of the Senate. The President of "
    "the Senate shall, in the Presence of the Senate an"
    "d House of Representatives, open all the Certifica"
    "tes, and the Votes shall then be counted. The Pers"
    "on having the greatest Number of Votes shall be th"
    "e President, if such Number be a Majority of the w"
    "hole Number of Electors appointed; and if there be"
    " more than one who have such Majority, and have an"
    " equal Number of Votes, then the House of Represen"
    "tatives shall immediately chuse by Ballot one of t"
    "hem for President; and if no Person have a Majorit"
    "y, then from the five highest on the List the said"
    " House shall in like Manner chuse the President. B"
    "ut in chusing the President, the Votes shall be ta"
    "ken by States, the Representation from each State "
    "having one Vote; A quorum for this purpose shall c"
    "onsist of a Member or Members from two thirds of t"
    "he States, and a Majority of all the States shall "
    "be necessary to a Choice. In every Case, after the"
    " Choice of the President, the Person having the gr"
    "eatest Number of Votes of the Electors shall be th"
    "e Vice President. But if there should remain two o"
    "r more who have equal Votes, the Senate shall chus"
    "e from them by Ballot the Vice President.\n\nThe C"
    "ongress may determine the Time of chusing the Elec"
    "tors, and the Day on which they shall give their V"
    "otes; which Day shall be the same throughout the U"
    "nited States.\n\nNo Person except a natural born C"
    "itizen, or a Citizen of the United States, at the "
    "time of the Adoption of this Constitution, shall b"
    "e eligible to the Office of President; neither sha"
    "ll any Person be eligible to that Office who shall"
    " not have attained to the Age of thirty five Years"
    ", and been fourteen Years a Resident within the Un"
    "ited States.\n\nIn Case of the Removal of the Pres"
    "ident from Office, or of his Death, Resignation, o"
    "r Inability to discharge the Powers and Duties of "
    "the said Office, the Same shall devolve on the Vic"
    "e President, and the Congress may by Law provide f"
    "or the Case of Removal, Death, Resignation or Inab"
    "ility, both of the President and Vice President, d"
    "eclaring what Officer shall then act as President,"
    " and such Officer shall act accordingly, until the"
    " Disability be removed, or a President shall be el"
    "ected.\n\nThe President shall, at stated Times, re"
    "ceive for his Services, a Compensation, which shal"
    "l neither be increased nor diminished during the P"
    "eriod for which he shall have been elected, and he"
    " shall not receive within that Period any other Em"
    "olument from the United States, or any of them.\n\n"
    "Before he enter on the Execution of his Office, h"
    "e shall take the following Oath or Affirmation:--\""
    "I do solemnly swear (or affirm) that I will faithf"
    "ully execute the Office of President of the United"
    " States, and will to the best of my Ability, prese"
    "rve, protect and defend the Constitution of the Un"
    "ited States.\"\n\nSection. 2.\n\nThe President shal"
    "l be Commander in Chief of the Army and Navy of th"
    "e United States, and of the Militia of the several"
    " States, when called into the actual Service of th"
    "e United States; he may require the Opinion, in wr"
    "iting, of the principal Officer in each of the exe"
    "cutive Departments, upon any Subject relating to t"
    "he Duties of their respective Offices, and he shal"
    "l have Power to grant Reprieves and Pardons for Of"
    "fences against the United States, except in Cases "
    "of Impeachment.\n\nHe shall have Power, by and wit"
    "h the Advice and Consent of the Senate, to make Tr"
    "eaties, provided two thirds of the Senators presen"
    "t concur; and he shall nominate, and by and with t"
    "he Advice and Consent of the Senate, shall appoint"
    " Ambassadors, other public Ministers and Consuls, "
    "Judges of the supreme Court, and all other Officer"
    "s of the United States, whose Appointments are not"
    " herein otherwise provided for, and which shall be"
    " established by Law: but the Congress may by Law v"
    "est the Appointment of such inferior Officers, as "
    "they think proper, in the President alone, in the "
    "Courts of Law, or in the Heads of Departments.\n\n"
    "The President shall have Power to fill up all Vaca"
    "ncies that may happen during the Recess of the Sen"
    "ate, by granting Commissions which shall expire at"
    " the End of their next Session.\n\nSection. 3.\n\n"
    "He shall from time to time give to the Congress In"
    "formation of the State of the Union, and recommend"
    " to their Consideration such Measures as he shall "
    "judge necessary and expedient; he may, on extraord"
    "inary Occasions, convene both Houses, or either of"
    " them, and in Case of Disagreement between them, w"
    "ith Respect to the Time of Adjournment, he may adj"
    "ourn them to such Time as he shall think proper; h"
    "e shall receive Ambassadors and other public Minis"
    "ters; he shall take Care that the Laws be faithful"
    "ly executed, and shall Commission all the Officers"
    " of the United States.\n\nSection. 4.\n\nThe Presi"
    "dent, Vice President and all civil Officers of the"
    " United States, shall be removed from Office on Im"
    "peachment for, and Conviction of, Treason, Bribery"
    ", or other high Crimes and Misdemeanors." };

static int seekset[] = {3,14,4096,159,4095,-2010,26,-1,53,5,89,5000,20000,79,323,0,4097, sizeof(article2), 52, sizeof(article2)-1 };
static int seekend[] = { 3, 14,  159,  20000,  5020,  1,  4096,  409,  4095, 20000,  100,  4097,  100,  5906, 0, 16,  sizeof(article2), 20, sizeof(article2)+1, 50, sizeof(article2)-1, 77, sizeof(article2)-2};
static int seekcur[] = {-10, 50, -13, 1000, -4000, 4000, -4099, 4099, 0, -1, 20000, -20000,
        sizeof(article2)};

int readFully(int fd, char buf[], int num){
    int total=0;
    char* p = buf;
    while(num>0){
        int nr = file_read(fd,p,num);
        if( nr == 0 )
            break;
        if( nr < 0 )
            return nr;
        p += nr;
        num -= nr;
        total += nr;
    }
    return total;
}
        
        
void sweet(){
    char buf[32];
    char buf2[32];
    int fd;
    int i,j,k;
    int rv;
    
    
    //additional tests
    int fd1,fd2,fd3;
    fd1 = file_open("article6.txt",0);
    fd2 = file_open("article4.txt",0);
    fd3 = file_open("article6.txt",0);
    if( fd1 < 0 || fd2 < 0 || fd3 < 0 ){
        kprintf("Could not open correctly!");
        return;
    }
    
    int theyAllMatch=1;
    while(1){
        ksprintf(debugMsg,"here on cycle:%d\n",cycle1++);
        logString(debugMsg);
        int nr1 = readFully(fd1,buf,27);
        if( nr1 < 0 ){
            kprintf("readFully wrong\n");
            return;
        }
        if( nr1 == 0 )
            break;
        int nr2 = readFully(fd2,buf2,27);
        if( nr2 <= 0 ){
            kprintf("wrongedy wrong wrong wrong\n");
            return;
        }
        for(i=0;i<nr1;++i){
            if( buf[i] != buf2[i] )
                theyAllMatch=0;
        }
        int nr3 = readFully(fd3,buf2,27);
        if( nr1 != nr3 ){
            kprintf("so very wrong\n");
            return;
        }
        for(i=0;i<nr1;++i){
            if( buf[i] != buf2[i] ){
                kprintf("wrong! Wrong! WRONG!\n");
                return;
            }
        }
        logString("\n");
    }

    if( theyAllMatch ){
        kprintf("They all match! It's a conspiracy, I tell you!\n");
        return;
    }
    
    if( 0 != file_close(fd1) || 0 != file_close(fd2) || 0 != file_close(fd3) ){
        kprintf("A close call...\n");
        return;
    }
    
    
    fd = file_open("article6.txt",0);
    if( fd < 0 ){
        kprintf("Could not open\n");
        return;
    }
    ksprintf(debugMsg,"fd:%d\n",fd);
    logString(debugMsg);
    
    const char* p = article6;
    int totalNumRead=0;
    while(1){
        int nr = file_read(fd,buf,23);
        if( nr < 0 || nr > 23){
            kprintf("Error: nr=%d\n",nr);
            return;
        }
        if( nr == 0 )
            break;
        for(i=0;i<nr;++i){
            if( *p != buf[i] ){
                kprintf("Error: Did not read correctly: At buf[%d]: Found %c, expected %c\n",
                    i,buf[i],*p);
                ksprintf(debugMsg,"buff=[%.*s]\n", 23, buf);
                logString(debugMsg);
                return;
            }
            p++;
            totalNumRead++;
            if( totalNumRead > sizeof(article6)-1 ){
                kprintf("Read too much: Read %d, expected %d at line %d\n",
                    totalNumRead, sizeof(article6)-1,__LINE__);
                return;
            }
        }
    }
    if( sizeof(article6)-1 != totalNumRead){
        kprintf("Didn't read correct amount: Expected %d, got %d at line %d\n",
            sizeof(article6)-1, totalNumRead, __LINE__);
        return;
    }
    file_close(fd);

    fd = file_open("article2.txt",0);
    if(fd<0){
        kprintf("Could not open article2.txt\n");
        return;
    }
    
    //////////////set
    for(i=0;i<sizeof(seekset)/sizeof(seekset[0]);i++){
        rv = file_seek(fd,seekset[i],SEEK_SET);
        if( seekset[i] < 0 ){
            if(rv >= 0 ){
                kprintf("Error: seek set succeeded with negative offset\n");
                return;
            }
        }
        else{
            if( rv < 0 ){
                kprintf("Seek set failed with positive offset\n");
                return;
            }
            else if( rv != seekset[i] ){
                kprintf("Seek set returned wrong value\n");
                return;
            }
            rv = file_read(fd,buf,32);
            if( seekset[i] >= sizeof(article2)-1 ){
                if( rv != 0 ){
                    kprintf("Read data, but should not have\n");
                    return;
                }
            }
            else{
                if( rv < 0 ){
                    kprintf("Read failed, but should not have\n");
                    return;
                }
                if( rv == 0 ){
                    kprintf("Read gave eof, but it should not have\n");
                    return;
                }
                if( seekset[i] + rv > sizeof(article2)-1 ){
                    kprintf("Returned too much data\n");
                    return;
                }
                for(j=0;j<rv;++j){
                    if( article2[seekset[i]+j] != buf[j] ){
                        kprintf("Read wrong contents on seek set\n");
                        return;
                    }
                }
            }
        }
    }
    
    
    ///////////////end
    for(i=0;i<sizeof(seekend) / sizeof(seekend[0]);i++){
        int sign;
        for(sign=-1;sign<=1;sign+=2){
            int offset = sizeof(article2)-1 + sign * seekend[i];
            rv = file_seek(fd,sign*seekend[i],SEEK_END);
            if( offset < 0 ){
                if(rv >= 0 ){
                    kprintf("Bad: seek end negative improperly succeeded\n");
                    return;
                }
            }
            else{
                if( rv < 0 ){
                    kprintf("seek end failed improperly\n");
                    return;
                }
                if( rv != offset ){
                    kprintf("seek end returned wrong value\n");
                    return;
                }
                rv = file_read(fd,buf,32);
                int max = sizeof(article2)-1 - offset;
                if( max < 0 )
                    max = 0;
                if( rv < 0 || rv > max ){
                    kprintf("Bad: Read wrong amount: %d; expected no more than %d\n",
                        rv,max);
                    return;
                }
                else if( rv == 0 && max != 0 ){
                    kprintf("Didn't read anything\n");
                    return;
                }
                
                for(j=0;j<rv;++j){
                    if( article2[offset+j] != buf[j] ){
                        kprintf("Read wrong contents on seek end\n");
                        return;
                    }
                }
            }
        }
    }    


    //////////cur
    for(i=0;i<sizeof(seekset)/sizeof(seekset[0]);i++){
        int m = sizeof(seekcur)/sizeof(seekcur[0]);
        for(j=0;j<m;++j){
            rv = file_seek(fd,seekset[i],SEEK_SET);
            if( seekset[i] < 0 ){
                if(rv>=0){
                    kprintf("Improperly succeeded on seek set to negative\n");
                    return;
                }
            }
            else{
                if( rv < 0 ){
                    kprintf("Failed cur/set on positive offset\n");
                    return;
                }
                if( rv != seekset[i] ){
                    kprintf("Wrong result on seek set before seek cur\n");
                    return;
                }
                
                rv = file_read(fd,buf,1);
                if( rv < 0 ){
                    kprintf("Read failed\n");
                    return;
                }
                if( seekset[i] >= sizeof(article2)-1){
                    if(rv != 0 ){
                        kprintf("Read got data but should not have\n");
                        return;
                    }
                }
                else{
                    if( rv != 1 ){
                        kprintf("Read 1 byte failed");
                        return;
                    }
                    if( article2[seekset[i]] != buf[0] ){
                        kprintf("Got wrong byte\n");
                        return;
                    }

                    rv = file_seek(fd,seekcur[j],SEEK_CUR);
                    if( seekset[i] + seekcur[j] +1 < 0 ){
                        if( rv >= 0 ){
                            kprintf("Bad: Seek to negative succeeded\n");
                            return;
                        }
                    }
                    else{
                        if( rv != seekset[i]+seekcur[j]+1 ){
                            kprintf("Seek to positive gave wrong return value: "
                                "%d expected %d\n", rv, seekset[i]+seekcur[j]);
                            return;
                        }
                        rv = file_read(fd,buf,sizeof(buf));
                        if( seekset[i] + seekcur[j] + 1 >= sizeof(article2)-1){
                            if( rv != 0 ){
                                kprintf("Read succeeded but should not have\n");
                                return;
                            }
                        }
                        else{
                            int numleft = sizeof(article2)-1 - (seekset[i]+seekcur[j]+1) ;
                            if( rv > sizeof(buf) || rv > numleft || rv == 0 ){
                                kprintf("Bad number read\n");
                                return;
                            }
                            for(k=0;k<rv;++k){
                                if( article2[seekset[i]+seekcur[j]+k+1] != buf[k] ){
                                    kprintf("Read wrong contents on seek cur: Expected %c, got %c\n",
                                        article2[seekset[i]+seekcur[j]+k+1], buf[k]);
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if( 0 != file_close(fd) ){
        kprintf("Could not close\n");
        return;
    }
    
    kprintf("\n"    
        "      \001\001\001\001\001\001\n"
        "   \001\001\001\001\001\001\001\001\001\001\001\001\n"
        "   \001\001\001 \001\001\001\001 \001\001\001\n"
        "  \001\001\001\001\001\001\001\001\001\001\001\001\001\001\n"
        "  \001\001\001\001\001\001\001\001\001\001\001\001\001\001\n"
        "   \001\001 \001\001\001\001\001\001 \001\001\n"
        "   \001\001\001      \001\001\001\n"
        "     \001\001\001\001\001\001\001\001\n"
        "\n");
}
