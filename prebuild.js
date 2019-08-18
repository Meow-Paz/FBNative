const fs=require("fs");
console.log("-Generating Build Info");
let count=parseInt(fs.readFileSync("core/fbbuildinfo.json").toString());
count++;
fs.writeFileSync("core/fbbuildinfo.json",count);
fs.writeFileSync("core/fbbuildinfo.h","#define BUILD \""+count+"\"");