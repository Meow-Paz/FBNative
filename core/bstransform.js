const original_cm=require("./BlockState.json");
const fs=require("fs");
let file="//Auto generated file.\n#include <memory>\n#include <vector>\n#include <tuple>\n\n";
file+="enum class nbt_properties={facing,half,hinge,open,powered,east,north,south,west,in_wall,shape,damage,occupied,part,has_bottle_0,has_bottle_1,has_bottle_2,variant,age,bites,color,level,up,triggered,power,snowy,seamless,eye,moisture,alt,flip,upper,contents,legacy_data,axis,enabled,has_record,check_decay,decayable,extended,type,short,mode,delay,locked,stage,nodrop,layers,wet,rotation,explode,attached,disarmed,suspended};\n";
file+="std::vector<std::tuple<unsigned char,std::string,std::vector<unsigned char>>> colormap={";
let first=true;
for(let i of original_cm.minecraft){
	if(!first){
		file+=",";
	}else{
		first=false;
	}
	file+="{"+i.data+",\""+i.name+"\",{"+i.color[0]+","+i.color[1]+","+i.color[2]+","+i.color[3]+"}}";
}
file+="};";
fs.writeFileSync("colortable.h",file);