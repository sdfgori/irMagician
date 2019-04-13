#include <fcntl.h>	//open
#include <sys/types.h>	//select(FD_ISSET,FD_ZERO,FD_SET)
#include <termios.h>	//tcdrain,cfsetspeed,cfmakeraw,tcflush,tcsetattr
#include <unistd.h>	//read,write,close,getopt,sleep

#include <string>
#include <vector>
#include <fstream>

#include "json.hpp"
using json = nlohmann::json;

class IR{
  private:
	int fd;      
	struct termios tio={0};				//シリアル通信設定
	bool opened;
	bool showDebug;
	fd_set readfs;						//監視対象登録用
	struct timespec ts400m;	
	
	void writedata(std::string str){
		int len;
		
		if(showDebug)printf("  write %s  len=%lu\n",str.c_str(),str.length());
		len=write(fd, str.c_str(), str.length());
		if(showDebug){
			if(0<=len)printf("  ==>len=%d\n",len);
			else printf("  ==>%d %s\n",errno,strerror(errno));
		}
		tcdrain(fd);					//送信完了を待つ
	}
	
	std::string readline(){
		unsigned char buf[255];			//バッファ
		int len;
		std::string str;
		struct timeval timeout;
		int res;
		
		if(opened==false) return str;

		timeout.tv_usec = 0;
		timeout.tv_sec = 1;				//タイムアウト1秒
		res = select(fd+1, &readfs, NULL, NULL, &timeout);//読み込みを待ち
		if(res!=0 && FD_ISSET(fd,&readfs)){		//読み込み可能データがある
			len = read(fd, buf, sizeof(buf));
			if (0 < len) {
				str.append((char*)buf,len);
				if(showDebug)printf("  <==%s  len=%lu\n",str.c_str(),str.length());
			}else if(len < 0)printf("  <==%d %s\n",errno,strerror(errno));
		}
		return str;
	}

	std::string readbyte(){
		unsigned char buf[3];            		 //バッファ
		int len;
		std::string str;
		struct timeval timeout;
		int res;
		
		if(opened==false) return str;
		
		tio.c_lflag = 0;				//非カノニカル入力
		tio.c_cc[VMIN] = 3;				//3文字読み込む
		tcsetattr( fd, TCSANOW, &tio );			//一時的に設定変更
		
		timeout.tv_usec = 0;
		timeout.tv_sec = 1;				//タイムアウト1秒
		res = select(fd+1, &readfs, NULL, NULL, &timeout);//読み込みを待ち
		if(res!=0 && FD_ISSET(fd,&readfs)){		//読み込み可能データがある
			len = read(fd, buf, sizeof(buf));
			if (0 < len) {
				str.append((char*)buf,len);
				if(showDebug)printf("  <==%s  len=%lu\n",str.c_str(),str.length());
			}
		}
		tio.c_lflag = ICANON;				//カノニカル入力
		tio.c_cc[VMIN] = 0;
		tcsetattr( fd, TCSANOW, &tio );			//設定を戻す

		return str;
	}

	std::string itos(int num){
		std::string str;
		int n = snprintf (nullptr, 0, "%-d", num);
		char* numc = new char[n + 1];
		snprintf( numc, n + 1, "%-d", num );
		str+=numc;
		delete[] numc;
		return std::move(str);
	}
  
	std::string formatstring(std::string prefix,int num,std::string suffix){
		std::string str=prefix;
		str+=itos(num);
		str+=suffix;
		return str;
	}

	std::string formatstring2(std::string prefix,int num,int num2,std::string suffix){
		std::string str=prefix;
		str+=itos(num);
		str+=",";
		str+=itos(num2);
		str+=suffix;
		return str;
	}

  public:
	IR(bool debug=false){
		opened = false;
		showDebug = debug;
		
		// /dev/ttyACM0 まれにttyACM1になったりするのでこれは使えない
		fd = open("/dev/irMagician", O_RDWR| O_NOCTTY | O_NONBLOCK);	// デバイスをオープンする
		if (fd < 0) {
			printf("open error\n");
			return;
		}
		
		tio.c_cflag += CREAD; 		//受信有効
		tio.c_cflag += CLOCAL;		//ローカルライン（モデム制御なし）
		tio.c_cflag += CS8;		//データビット:8bit
		tio.c_cflag += 0;		//ストップビット:1bit
		tio.c_cflag += 0;		//パリティ:None

		cfsetspeed(&tio, B9600);	//通信速度の設定

		cfmakeraw(&tio);		//RAWモード(バイナリデータ)

		tio.c_lflag = ICANON;		//カノニカル入力
		tio.c_cc[VMIN] = 0;		//最低限読み込む文字数※使用しない
		tio.c_cc[VTIME]= 0;		//タイムアウト値 0秒　※使用しない

		tcflush(fd, TCIFLUSH);		//送受信データをクリア
		tcsetattr( fd, TCSANOW, &tio );	//シリアルポートの属性を変更する TCSANOW=即座に変更される

		FD_ZERO(&readfs);		//クリア
		FD_SET(fd, &readfs);		//fdを追加
		
		ts400m.tv_sec = 0;
		ts400m.tv_nsec =400000000;	//400ミリ秒
		opened=true;
	}
	
	~IR(){
		if(opened==false) return;
			close(fd);  
	}
   
	void showversion(){
		if(opened==false) return;
		writedata("V\r\n");		//バージョン表示
		std::string str = readline();
		printf("%s\n",str.c_str());
	}

	void capture(){
		if(opened==false) return;
		printf("Capturing IR....\n");
		writedata("c\r\n");		//赤外線リモコン信号を取得
		sleep(3);
		std::string str = readline();
		printf("%s\n",str.c_str());
	}
	
	void savefile(std::string file){
		if(opened==false) return;
		printf("Saving IR data to %s ...\n",file.c_str());
		
		writedata("I,1\r\n");				//赤外線信号の変化点 (L/Hの切り替わり)の数
		std::string str = readline();
		int numlen =std::stoi(str, nullptr, 16);	//10進の数値←16進文字列
		if(showDebug)printf("data-length=%d\n",numlen); 
		
		writedata("I,6\r\n");				//postScalerの値
		std::string str2 = readline();
		int numpostScaler =std::stoi(str2, nullptr, 10);//数値に変換
		if(showDebug)printf("postScaler=%d\n",numpostScaler); 
		
		std::vector<int >v;				//配列
		
		for(int idx=0;idx<numlen;idx++){
			int bank = idx /64;
			int pos  = idx % 64;
			if(showDebug)printf("%d-%d/%d\n",idx,bank,pos);
			if(pos==0){
				std::string str3=formatstring("b,",bank,"\r\n");	//メモリのバンク設定 0-9
				writedata(str3);
			}
			std::string str4=formatstring("d,",pos,"\r\n");		//メモリの表示
			writedata(str4);
			std::string str5 = readbyte();
			int num3 =std::stoi(str5, nullptr, 16);			//10進の数値←16進文字列
			if(showDebug)printf("  data=%d\n",num3);
			v.push_back(num3);						//配列に登録
		}
		
		if(showDebug)printf("data-length=%lu\n",v.size());

		json j2(v);			//配列データ
		json j;				//jsonデータ
		j["format"] = "raw";
		j["freq"] = 38;
		j["data"] = j2;
		j["postscale"] = numpostScaler;
		if(showDebug){
			std::string s = j.dump(); 
			printf("%s\n",s.c_str());
		}
		
		std::ofstream o(file);		//ファイルに出力する
		o << j << std::endl;
		printf("Done !\n");		
	}
	
	void loadfile(std::string file){
		if(opened==false) return;
		printf("Playing IR with %s ...\n",file.c_str());
		
		std::ifstream i(file.c_str());	//ファイルから読み込む
		json j;
		i >> j;				//jsonデータ
		
		if(showDebug){
			std::string s = j.dump(); 
			printf("%s\n",s.c_str());
		}

		json j2=j["data"];						//配列データ
		int numlen =j2.size();
		if(showDebug)printf("data-length=%d\n",numlen);
		std::string str=formatstring("n,",numlen,"\r\n");		//赤外線信号の変化点 ２バイトの符号なし整数
		writedata(str);
		std::string strr = readline();
		
		int numpostScaler =j["postscale"];
		if(showDebug)printf("postscale=%d\n",numpostScaler);
		std::string str2=formatstring("k,",numpostScaler,"\r\n");	//postScalerの値 1-255
		writedata(str2);
		std::string str2r = readline();
		
		for(int idx=0;idx<numlen;idx++){
			int bank = idx /64;
			int pos  = idx % 64;
			if(showDebug)printf("%d-%d/%d\n",idx,bank,pos);
			if(pos==0){
				std::string str3=formatstring("b,",bank,"\r\n");//メモリのバンク設定 0-9
				writedata(str3);
			}
			std::string str4=formatstring2("w,",pos,j2[idx],"\r\n");//赤外線データの書き込み
			writedata(str4);
		}
	}
	
	void play(){
		if(opened==false) return;
		printf("Playing IR...");
		writedata("p\r\n");						//ローカルに格納されている赤外線信号の再生
		std::string str = readline();
		printf("%s\n",str.c_str());
	}

	//
	void playnumber(std::string number,std::string suffix=".json"){
		if(opened==false) return;
		std::string str = number;
		for(int idx=0;idx<str.length();idx++){		//文字列を1字ずつ取り出してファイルとして扱う
			std::string str2=str.substr(idx,1);
			str2+=suffix;
			printf("%s\n",str2.c_str());
			loadfile(str2.c_str());			//ファイルを読みだしてデータを送信
			play();					//赤外線信号の再生
			nanosleep(&ts400m,NULL);		//待ち
		}
	}
};
