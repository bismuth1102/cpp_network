#include <iostream>
#include <fstream>
#include <libgen.h>
#include <boost/algorithm/string.hpp>
#include <unistd.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>  
#include <time.h>
#include <vector>
#include <future>
#define BUF_SIZE 8192

using namespace std;
using namespace boost::asio;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

duration<double> throughput;

class DHTClient 
{
public:

  // DHTClient():socket_(create_socket("localhost", "40300")){socket_.close();}
  DHTClient(bool test_mode, string port_){   // initialize with DHTClient
    ifstream in("DHTConfig");
    if (in.is_open())
    {
      for (string str; getline(in, str) ;)
      { 
        if (!test_mode && str.find("IP")!=str.npos)
        {
          str = str.substr(str.find("=")+1);
          boost::split(server_list_, str, boost::is_any_of(","));
        }
      }
      in.close();
    }
    num_server_ = test_mode ? 0 : server_list_.size();
    if (test_mode)
    {
      server_list_.push_back("localhost");
      socket_.push_back(create_socket(server_list_[0], port_));
    }

    for(vector<string>::iterator itor = server_list_.begin();itor!=server_list_.end();++itor){
        cout<<*itor<<endl;
    }

    for (int i = 0; i < num_server_; ++i)
    {
      socket_.push_back(create_socket(server_list_[i], port_));
    }
  }

  boost::asio::ip::tcp::socket create_socket(std::string hostname, std::string port) {
    io_service io_service;
    ip::tcp::resolver resolver(io_service);
    ip::tcp::resolver::query query(hostname, port);
    ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    ip::tcp::socket socket(io_service);
    try{
      connect(socket, endpoint_iterator);
    }catch (std::exception &e){
      // std::cerr << e.what() << std::endl;
      cout << hostname << " is offline" << endl;
      exit(1);
    }
    return socket;
  }

  string write_read_buffer(boost::asio::ip::tcp::socket &socket, string data){
    using namespace std;
    using namespace boost::asio;

    // track connection duration, bytes transmitted
    size_t xmitBytes = 0;

    //ready to recieve data from server
    size_t recd = 0, position;
    unsigned char buf[BUF_SIZE];
    // vector<unsigned char> buf;
    size_t len;
    stringstream stream;
    string str_buf;
    boost::system::error_code ignored_error;
    write(socket, buffer(data), ignored_error);
    xmitBytes += data.length();

    str_buf = "";
    while (true)
    {
      len = socket.read_some(boost::asio::buffer(buf), ignored_error);

      for (int i = 0; i < len; ++i)
      {
        stream << buf[i];
      }

      str_buf = stream.str();
      position = str_buf.find("\n");
      if ( position < str_buf.length() )
      {
        recd = stoi(str_buf.substr(0, position));
        break;
      }
    }

    stream.str("");

    while(str_buf.length()<recd+1+to_string(recd).length())
    {
      len = socket.read_some(boost::asio::buffer(buf), ignored_error);
      for (int i = 0; i < len; ++i)
      {
        stream << buf[i];
      }
      str_buf = str_buf + stream.str();
      stream.str("");
    }

    position = str_buf.find("\n");
    str_buf = str_buf.substr(position+1);

    return str_buf;
  }

  string send_message(string data, int index = 0){
    string str_response;

    auto starttime = high_resolution_clock::now();
    try{
      // auto socket = create_socket(server_name, port);
      str_response = write_read_buffer(socket_[index], data);
    }catch (std::exception &e){
      std::cerr << e.what() << std::endl;
      return "RW Error";
    }
    auto endtime = high_resolution_clock::now();

    duration<double> time_span = duration_cast<duration<double>>(endtime - starttime);

    throughput = throughput + time_span;

    return str_response;
    cout << "Time span: " << time_span.count() << endl;
  }

  void close_socket(){

    for (int i = 0; i < server_list_.size(); ++i)
    {
      send_message("SHOW\n", i);
      socket_[i].close();
    }
  }

  int get_num_server_(){
    return num_server_;
  }

private: 
  vector<ip::tcp::socket> socket_;
  vector<string> server_list_;
  int num_server_;
  string port_;
};

class dataGenerator
{
public:
  dataGenerator(int num_server):key_(""),num_server_(num_server){}

  string command_test(){
    string data, key, value;
    cout<<"Client: ";
    getline(cin, data);

    // ******* PUT *******
    if (data == "PUT")
    {
      cout << "KEY:";
      getline(cin, key);
      cout << "VAL:";
      getline(cin, value);
      // PUT\nkey\nvalue
      data = "PUT\n" + key + "\n" + value;
    }
    // ******* GET *******
    else if (data == "GET")
    {
      cout << "KEY:";
      getline(cin, key);
      // GET\nkey
      data = data + "\n" + key;
    }
    // ******* SHOW / INIT *******
    else if (data == "SHOW" || data == "INIT")
    {
      data = data + "\n";
    }
    // ******* EXIT *******
    else if (data == "EXIT")
    {
      return "EXIT";
    }
    return data;
  }

  string command(int cmd, int length_key){
    int length_value;
    string data, key, value;
    length_value = rand()%3 + 1;

    key = random_str(length_key);
    value = random_str(length_value);
    key_ = key;

    if (cmd==1)
    { //put
      data = "PUT\n" + key + "\n" + value;
      cout << "PUT " << key << " " << value << endl;
    }
    // else if (cmd==2)
    // { //mul_put
    //   data = "PUT\n" + key + "\n" + value + ",";
    //   cout << "PUT " << key << " " << value << endl;
    //   key = random_str(length_key);
    //   value = random_str(length_value);
    //   data += "PUT\n" + key + "\n" + value + ",";
    //   cout << "PUT " << key << " " << value << endl;
    //   key = random_str(length_key);
    //   value = random_str(length_value);
    //   data += "PUT\n" + key + "\n" + value;
    //   cout << "PUT " << key << " " << value << endl;

    // }
    else if (cmd==3)
    { //get
      data = "GET\n" + key;
      cout << "GET  " << key << endl;
    }
    return data;
  }

  string random_str(int length){
    stringstream ss;
    string random_str;
    int flag;
    for (int i = 0; i < length; i++)
    {
      flag = rand() % 2;
      switch (flag)
      {
      // case 0:
      // ss << char('A' + rand() % 26);
      // break;
        case 0:
        ss << char('a' + rand() % 26);
        break;
        case 1:
        ss << char('0' + rand() % 10);
        break;
        default:
        ss << 'x';
        break;
      }
    }

    ss >> random_str;
    return random_str;
  }

  int pickServer(){
    int id_server;
    hash<string> h;

    size_t n = h(key_);
    id_server = n % num_server_;
    return id_server;
  }

private:
  string key_;
  int num_server_;
};


int client_main(string port="40300", int num_command=10, int key_len=3, bool test_mode=false) {
  string data;
  //string server_name = "localhost";

   // START
  auto starttime = high_resolution_clock::now();
  
  int numServer;
  DHTClient client(test_mode, port);
  numServer = client.get_num_server_();
  dataGenerator dataGen(numServer);
  /**********************************************/
  /* test mode: only communicate with localhost */
  /**********************************************/
  if (test_mode)      
  {  
    while(true){
      data = dataGen.command_test(); 
      if (data == "EXIT")
      {
        break;
      } 
      cout << "SERVER: " << client.send_message(data) << endl;;
    }
  }
  /**********************************************/
  /********** communicate with the DHT **********/
  /**********************************************/
  else      
  {  
    int cmd; // 0: put; 1: mul_put; 2: get
    int serverID;
    string keyID;
    
    srand((unsigned)time(NULL));

    for (int i = 0; i < num_command; ++i)
    {
      int temp = rand()%10+1;
      if (temp<=2) cmd = 1; //put
      else if(temp<=4) cmd = 2; //mul_put
      else cmd = 3; //get

      if (cmd==1)
      {
        data = dataGen.command(cmd, key_len);
        serverID = dataGen.pickServer();
        string str1, str2;

        while(true){
          str1 = client.send_message("LOCK\n", serverID);
          str2 = client.send_message("LOCK\n", (serverID+1)%numServer);

          cout << "Server" << serverID << ":" << str1 << " " << "Server" << (serverID+1)%numServer << ":" << str2 << endl;

          if ( str1 == "GO" && str2 == "GO")
          {
            cout << "Server" << serverID << " " << client.send_message(data, serverID) << endl;
            cout << "Server" << (serverID+1)%numServer << " " << client.send_message(data, (serverID+1)%numServer) << endl;
            client.send_message("UNLOCK\n", serverID);
            client.send_message("UNLOCK\n", (serverID+1)%numServer);
            break;
          }
          else if (str1 == "GO"){
            client.send_message("UNLOCK\n", serverID);
          }
          else if (str2 == "GO"){
            client.send_message("UNLOCK\n", (serverID+1)%numServer);
          }
        }
        
      }
      else if (cmd==2)
      {
        vector<string> dataset; 
        boost::split(dataset, data, boost::is_any_of(","));
        for (int i = 0; i < 3; ++i)
        {
          data = dataset[i];
          string str1, str2;

          while(true){
            str1 = client.send_message("LOCK\n", serverID);
            str2 = client.send_message("LOCK\n", (serverID+1)%numServer);

            cout << "Server" << serverID << ":" << str1 << " " << "Server" << (serverID+1)%numServer << ":" << str2 << endl;

            if ( str1 == "GO" && str2 == "GO")
            {
              cout << "Server" << serverID << " " << client.send_message(data, serverID) << endl;
              cout << "Server" << (serverID+1)%numServer << " " << client.send_message(data, (serverID+1)%numServer) << endl;
              client.send_message("UNLOCK\n", serverID);
              client.send_message("UNLOCK\n", (serverID+1)%numServer);
              break;
            }
            else if (str1 == "GO"){
              client.send_message("UNLOCK\n", serverID);
            }
            else if (str2 == "GO"){
              client.send_message("UNLOCK\n", (serverID+1)%numServer);
            }
          }
        }
      }
      else{
        cout << client.send_message(data, serverID) << endl;
      }
      
    }
  }
  cout << "Total execute time: " << throughput.count() << endl;

  auto endtime = high_resolution_clock::now();
  duration<double> total_time = duration_cast<duration<double>>(endtime - starttime);
  cout << "Total time: " << total_time.count() << endl;

  client.close_socket();

  ofstream throughput_file("throughput", ios::app);
  ofstream latency_file("latency", ios::app);

  throughput_file << double(num_command)/throughput.count() << endl;
  latency_file << total_time.count()*1000/double(num_command) << endl;

  throughput_file.close();
  latency_file.close();

  return 0;
}

int main(int argc, char *argv[]){
  string port = "40300";
  bool test_mode = false;
  //string server_name = "localhost";
  int num_command = 10, key_len = 3;

  int o;
  while ((o = getopt(argc, argv, "p:c:l:t")) != -1) {
    switch (o) {
      case 'p':
      port = string(optarg);
      break;
      case 'c':
      num_command = atoi(optarg);
      break;
      case 'l':
      key_len = atoi(optarg);
      break;
      case 't':
      test_mode = true;
      break;
      default:
      break;
    }
  }

  boost::asio::thread_pool tp(3);

  for(int i=0; i<5; ++i) {
    boost::asio::post(tp, boost::bind(client_main, port, num_command, key_len, test_mode));
  }

  tp.join();

  return 0;

}
