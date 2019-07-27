#include <functional>
#include <memory>
#include <map>
#include <vector>
#include "algorithms.h"
#include <fstream>
#include <io/izlibstream.h>
#include <io/stream_reader.h>
#include <nbt_tags.h>
#include "paletteresolver.h"
#include <png.h>
#include "colortable.h"
#include "../fbmain.h"
#include "../jsoncpp-1.8.4/include/json/json.h"
#include <math.h>
#include "fbscript.h"


class FBScriptBuildMethods {
private:
	static unsigned int getMin(unsigned int *arr,int length){
		unsigned int min = arr[0];
		for(int i = 1; i < length; i++) {
			//unsigned char cur = arr[i];
			if(arr[i] < min){min = arr[i];}
		}
		return min;
	}

	static int indexOfL(unsigned int val,unsigned int*List,int length){
		for(int i=0;i<length;i++){
			if(List[i]==val){	
				return i;
			}
		}
		return -1;
	}

	static Block *get_color(unsigned char r, unsigned char g,unsigned char b) {
		int r1,g1,b1;
		Json::Value rootc;
		Json::Reader reader;
		reader.parse(colortable_json,rootc);
		const int size=rootc.size();
		std::map<unsigned short,unsigned short> list;
		for(unsigned short i=0;i<size;i++){
			r1=r-rootc[i]["color"][0].asInt();
			g1=g-rootc[i]["color"][1].asInt();
			b1=b-rootc[i]["color"][2].asInt();
			list[i]=sqrt((r1 * r1) + (g1 * g1) + (b1 * b1));
		}
		unsigned short min=list[0];
		unsigned short index=0;
		for(unsigned short i=1;i<size;i++){
			if(list[i] < min){min = list[i];index=i;}
		}
		Block *block=new Block(rootc[index]["name"].asString(),(unsigned char)rootc[index]["data"].asInt());
		return block;
	}

	static csession *draw(std::vector<Block*> list,unsigned int w,unsigned int h,int xx,int yy,int zz){
		int x=xx,y=yy,z=zz+h;
		csession *rsl=new csession();
		int max = w + x;
		int min = x;
		int t = 0;
		while(1){
			if(x == max){
				z = z - 1;
				x = min;
			}
			x+=1;

			rsl->push(x,y,z,list[t]->name,list[t]->data);

			t++;
			if(t == list.size()){
				break;
			}
		}
		return rsl;
	}

	static float ColorDistance(Color color1,Color color2){
		int r1=color1.r,g1=color1.r,b1=color1.b;
		int r2=color2.r,g2=color2.r,b2=color2.b;
		int rmean=(r1+r2)>>1;
		int red=r1-r2;
		int green=g1-g2;
		int blue=b1-b2;
		float result=(((512+rmean)*red*red)>>8) + 4*green*green + (((767-rmean)*blue*blue)>>8);
		return sqrt(result);
	}

	static int findMin(std::vector<float> t){
		int min=t[0];
		int index=0;
		for(int n=1;n<t.size();n++){
			if(t[n]<min){
				min=t[n];
				index=n;
			}
		}
		return index;
	}

	static Block *getBlock(Color c){
		std::vector<float> list;
		Json::Value rootc;
		Json::Reader Jreader;
		Jreader.parse(colortable_json,rootc);
		const int size=rootc.size();
		for(unsigned short i=0;i<size;i++){
			list.push_back(ColorDistance(c,Color(rootc[i]["color"][0].asInt(),rootc[i]["color"][1].asInt(),rootc[i]["color"][2].asInt())));
		}
		int min=findMin(list);
		return new Block(rootc[min]["name"].asString(),rootc[min]["data"].asInt());
	}
public:
	static csession *Paint(int x, int y, int z,argInput input,FastBuilderSession *fbsession){
		FILE *img=fopen(input.path.c_str(),"r");
		if(!img){
			fbsession->sendText("Non-Exist file.");
			return nullptr;
		}
		unsigned char sig[8];
		fread(sig,1,8,img);
		if(!png_check_sig(sig,8)){
			fbsession->sendText("Not a png file.");
			fclose(img);
			return nullptr;
		}
		png_structp png=png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		png_infop info=png_create_info_struct(png);
		png_init_io(png,img);
		png_set_sig_bytes(png,8);
		png_read_info(png,info);

		png_uint_32 width = png_get_image_width(png, info);
		png_uint_32 height = png_get_image_height(png, info);
		png_uint_32 bitDepth = png_get_bit_depth(png, info);
		png_uint_32 channels = png_get_channels(png, info);

		png_bytep* rowPtrs=new png_bytep[height];
		unsigned long dataSize = width * height * bitDepth * channels / 8;
		unsigned char* data = new unsigned char[dataSize];
		const unsigned int stride = width * bitDepth * channels / 8;
		for (size_t i = 0; i < height; ++i) {
			png_uint_32 q = (height - i - 1) * stride;
			rowPtrs[i] = (png_bytep)data + q;
		}
		png_read_image(png, rowPtrs);

		std::vector<unsigned char> _d={0,0,0};
		int _dlast=0;
		std::vector<Block*> BuildList;

		for (int i = 0 ; i < width*height*4; i++){
			_d[_dlast]=data[i];
			_dlast++;
			if(_dlast==4/*i != 0 && (i + 1) % 4 == 0*/){
				BuildList.push_back(getBlock(Color(_d[0],_d[1],_d[2])));
				_dlast=0;
			}
		}
		
		csession *ss=draw(BuildList, width, height, x,y,z);
		delete[] rowPtrs;
		delete[] data;
		fclose(img);
		return ss;
	}

	static csession *buildNBT(int x, int y, int z,argInput input,FastBuilderSession *fbsession){
		try{
		std::ifstream file(input.path,std::ios::binary);
		zlib::izlibstream nbtz(file);
		auto rootpair=nbt::io::read_compound(nbtz);
		nbt::tag_list palette=rootpair.second->at("palette").as<nbt::tag_list>();
		std::map<unsigned int,Block*> resolvedPalettes;
		unsigned int _end=0;
		for(nbt::value &item:palette){
			std::string name=static_cast<const std::string&>(item.at("Name"));
			unsigned char data=0;
			name.replace(name.begin(),name.begin()+10,"");
			std::string::size_type st=name.find("door");
			std::string::size_type st_=name.find("trapdoor");
			if(st!=std::string::npos&&st_==std::string::npos){
				if(static_cast<const std::string&>(item.at("Properties").at("half"))=="upper"){
					resolvedPalettes[_end]=nullptr;
					_end++;
					continue;
				}
			}
			Json::Reader reader;
			Json::Value root;
			reader.parse(palette_json,root);
			if(root["minecraft"][name].isArray()){
				int size=root["minecraft"][name].size();
				bool ok=true;
				for(int i=0;i<size;i++){
					ok=true;
					Json::Value::Members memb=root["minecraft"][name][i]["block_state"].getMemberNames();
					for(auto it=memb.begin();it!=memb.end();it++){
						if(*it!="shape"&&item.at("Properties").as<nbt::tag_compound>().has_key(*it)&&(static_cast<const std::string&>(item.at("Properties").at(*it))!=root["minecraft"][name][i]["block_state"][*it].asString())){
							//std::cout<<name<<std::endl;
							//std::cout<<static_cast<const std::string&>(item.at("Properties").at(*it))<<"||"<<root["minecraft"][name][i]["block_state"][*it].asString()<<std::endl;
							ok=false;
							break;
						}
					}
					if(ok){
						data=root["minecraft"][name][i]["data_value"]["meta"].asInt();
						break;
					}
				}
			}
			resolvedPalettes[_end]=new Block(name,data);
			_end++;
		}
		nbt::tag_list blocks=rootpair.second->at("blocks").as<nbt::tag_list>();
		csession *ssr=new csession(true);
		for(nbt::value &item:blocks){
			Block *block=resolvedPalettes[static_cast<int64_t>(item.at("state"))];
			if(block==nullptr)continue;
			nbt::tag_list pos=item.at("pos").as<nbt::tag_list>();
			ssr->push(x+static_cast<int64_t>(pos[0]),y+static_cast<int64_t>(pos[1]),z+static_cast<int64_t>(pos[2]),block);
		}
		return ssr;
		}catch(std::exception e){
			return nullptr;
		}
	}
};

void initFBS(Algorithms *alg){
	alg->registerCommand("paint",&FBScriptBuildMethods::Paint);
	alg->registerCommand("NBT",&FBScriptBuildMethods::buildNBT);
}