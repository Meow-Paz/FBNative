#include <string>
#include <iostream>
#include <App.h>
#include <unistd.h>
#include <pthread.h>

void onMsg(void *,std::string);
void onConnection(void *);
void fbinitDone();

void _send(void *cl,std::string msg){
	auto *client=(uWS::WebSocket<false,true>*)cl;
	client->send(std::string_view(msg),uWS::OpCode::TEXT);
}

void onCon_newThread(void *ws){
	auto *rws=(uWS::WebSocket<false,true>*)ws;
	onConnection((void*)ws);
}

struct onMsgThreadStruct {
	void *ws;
	std::string_view msg;
};

void onMsg_newThread(void *strc){
	try{
	struct onMsgThreadStruct* sst=(struct onMsgThreadStruct*)strc;
	onMsg(sst->ws,std::string(sst->msg));
	}catch(const std::bad_alloc& err){}
}

void createWebsocketServer(unsigned short port){
	struct PerSocketData {
			
	};
	uWS::App().ws<PerSocketData>("/*", {
		.compression=uWS::DISABLED,
		.maxPayloadLength=2048,
		.idleTimeout=3600,
		.open=[](auto *ws,auto *req){
			pthread_t ptt;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
			pthread_create(&ptt,&attr,(void *(*)(void*))onCon_newThread,(void*)ws);
		},
		.message=[](auto *ws,std::string_view message,uWS::OpCode opcode){
			struct onMsgThreadStruct tst={0};
			tst.ws=(void*)ws;
			tst.msg=message;
			pthread_t ptt;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
			pthread_create(&ptt,&attr,(void *(*)(void*))onMsg_newThread,(void*)&tst);
		},
		.drain=[](auto *ws){},
		.ping=[](auto*ws){},
		.pong=[](auto*ws){},
		.close=[](auto *ws, int code, std::string_view message){
			
		}
	}).listen(port,[](auto *tk){
		if(tk){
			fbinitDone();
		}
	}).run();
}