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
#include <tuple>


class FBScriptBuildMethods {
private:
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
		const int size=colormap.size();
		unsigned char data;
		std::string bname;
		std::vector<unsigned char> color;
		for(unsigned short i=0;i<size;i++){
			std::tie(data,bname,color)=colormap[i];
			list.push_back(ColorDistance(c,Color(color[0],color[1],color[2])));
		}
		int min=findMin(list);
		std::tie(data,bname,color)=colormap[min];
		return new Block(bname,data);
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