#include <string>
using namespace std;
typedef unsigned int uint32;

class SMSModule {
    private:
        string receivers[4];
        int n_recievers;

    public:
        SMSModule(); //constructor
        //recieves the phone numbers of the recievers
        void getRecievers(); 
        // sends an alert to the recievers
        void sendSMS(string message);
};