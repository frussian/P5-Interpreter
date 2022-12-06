#include <iostream>
#include <set>

#include "p5_common.h"
#include "p5_errors.h"

#include "Assembler.h"
#include "LabelTable.h"
#include "Lexer.h"
#include "addr_ops.h"

std::unordered_map<std::string, P5::ins_t> Assembler::instr;
std::unordered_map<std::string, P5::ins_t> Assembler::sp_table;
std::unordered_map<P5::ins_t, Assembler::OpCodeInfo> Assembler::op_codes;

//"Pascal Implementation" Book
//https://homepages.cwi.nl/~steven/pascal/book/pascalimplementation.html

/*
*    maxstr -> ---------------------                                           *
*              | Constants         |                                           *
*        cp -> ---------------------                                           *
*              | Dynamic variables |                                           *
*        np -> ---------------------                                           *
*              | Free space        |                                           *
*        sp -> ---------------------                                           *
*              | Stack             |                                           *
*              ---------------------                                           *
*              | Code              |                                           *
*              ---------------------                                           *
*                                     
*/

Assembler::Assembler(P5::store_t &store, std::string filename):
	store(store),
	filename(std::move(filename)) {
	lb_table = new LabelTable(store);
	lexer = new Lexer(this->filename);
}

Assembler::~Assembler() {
	delete lb_table;
	delete lexer;
}

void Assembler::load() {
	std::cout << "loading " << filename << std::endl;
	pc = 0;
	cp = P5::max_store-1;
	lexer->start();
	generate();
	pc_top = pc;
	pc = 0;
	dump();
	generate();
	dump();
}

void Assembler::dump() {
	printf("PROGCODE\n");
	for (P5::addr_t i = 0; i < pc_top;) {
		auto op_code = get_addr<P5::ins_t>(store, i);
		auto start = i;
		i += sizeof(P5::ins_t);
		auto info = op_codes[op_code];
		
		// auto op_name = instr[op_code];
		printf("%d ", op_code);
		if (info.has_p) {
			printf("%d ", get_addr<P5::lvl_t>(store, i));
			i += sizeof(P5::lvl_t);
		}
		
		
		if (info.q_len != 0) {
			P5::addr_t q = 0;
			get_addr_by_ptr(store, i, (unsigned char*)&q, info.q_len);
			i += info.q_len;
			printf("%d ", q);
		}
		printf("[%d bytes]\n", i-start);
	}
	printf(".\n.\n.\n");
	printf("CONSTANTS\n");
	for (P5::addr_t i = cp+1; i < P5::max_store; i++) {
		printf("%d\n", store[i]);
	}
}

void Assembler::generate() {
	bool again = true;
	while (again) {
		if (lexer->is_eof()) {
			P5_ERR("unexpected end of file\n");
		}
		printf("assembling line %d\n", lexer->line_num());
		char c = lexer->get<char>();
		switch (c) {
			//comment
			case 'i': {
				printf("i");
				lexer->get_line();
				break;
			}
			case 'l': {
				int x = lexer->get<int>();
				c = lexer->get<char>();
				int lbl_val;
				if (c == '=') {
					lbl_val = lexer->get<int>();
				} else {
					lbl_val = pc;
				}
				printf("label %d %d %d\n", x, c, lbl_val);
				lb_table->update(x, lbl_val);
				lexer->get_line();
				break;
			}
			case 'q': {
				again = false;
				lexer->get_line();
				break;
			}
			//source line, TODO: process this
			case ':': {
				lexer->get_line();
				break;
			}
			case ' ': {
				assemble();
				lexer->get_line();
				break;
			}
			default: {
				P5_ERR("invalid char %c(%d)\n", c, c);
			}
		}
	}
};

void Assembler::assemble() {
	std::string name = lexer->get<std::string>();
//	std::string name = "tes";
	printf("instr %s\n", name.c_str());
	auto ins_it = instr.find(name);
	if (ins_it == instr.end()) {
		P5_ERR("illegal instruction: %s\n", name.c_str());
	}
	auto op_code = ins_it->second;
	printf("opcode: %d\n", op_code);
	switch (op_code) {
		//lod,str,lda,lip
		case 0:
		case 105:
		case 106:
		case 107:
		case 108:
		case 109:
		case 2:
		case 70:
		case 71:
		case 72:
		case 73:
		case 74:
		case 4:
		case 120: {
			auto p = lexer->get<P5::lvl_t>();
			auto q = lexer->get<P5::addr_t>();
			store_pc(op_code);
			store_pc(p);
			store_pc(q);
			break;
		}
		//cup
		case 12: {
			auto p = lexer->get<P5::lvl_t>();
			auto q = label_search();
			store_pc(op_code);
			store_pc(p);
			store_pc(q);
			break;
		}
		//mst, cip
		case 11:
		case 113: {
			auto p = lexer->get<P5::lvl_t>();
			store_pc(op_code);
			store_pc(p);
			break;
		}
		//equm,neqm,geqm,grtm,leqm,lesm
		case 142:
		case 148:
		case 154:
		case 160:
		case 166:
		case 172:
		//lao,ixa,mov,dmp,swp
		case 5:
		case 16:
		case 55:
		case 117:
		case 118:
		//ldo,sro,ind,inc,dec
		case 1:
		case 65:
		case 66:
		case 67:
		case 68:
		case 69:
		case 3:
		case 75:
		case 76:
		case 77:
		case 78:
		case 79:
		case 9:
		case 85:
		case 86:
		case 87:
		case 88:
		case 89:
		case 10:
		case 90:
		case 91:
		case 92:
		case 93:
		case 94:
		case 57:
		case 100:
		case 101:
		case 102:
		case 103:
		case 104: {
			auto q = lexer->get<P5::addr_t>();
			store_pc(op_code);
			store_pc(q);
			break;
		}
		//pck, upk
		case 63:
		case 64: {
			auto q = lexer->get<P5::addr_t>();
			auto q1 = lexer->get<P5::addr_t>();
			store_pc(op_code);
			store_pc(q);
			store_pc(q1);
			break;
		}
		//ujp,fjp,xjp,lpa,tjp
		case 23:
		case 24:
		case 25:
		case 119: 
		//ents,ente
		case 13:
		case 173: {
			auto q = label_search();
			store_pc(op_code);
			store_pc(q);
			break;
		}
		//ipj,lpa
		case 112:
		case 114: {
			auto p = lexer->get<P5::lvl_t>();
			auto q = label_search();
			store_pc(op_code);
			store_pc(p);
			store_pc(q);
			break;

		}
		//csp
		case 15: {
			auto sp_name = lexer->get<std::string>();
			auto sp_it = sp_table.find(sp_name);
			if (sp_it == sp_table.end()) {
				P5_ERR("undefined standard procedure/function %s\n", sp_name.c_str());
			}
			store_pc(op_code);
			store_pc((P5::addr_t)sp_it->second);
			break;
		}
		//ldc
		case 123: {
			auto i = lexer->get<int>();
			store_pc(op_code);
			if (pc + sizeof(int) > cp) {
				P5_ERR("Program code overflow\n");
			}
			store_pc(i);
			break;
		}
		case 124: {
			auto r = lexer->get<double>();
			store_pc(op_code);
			if (pc + sizeof(double) > cp) {
				P5_ERR("Constant table overflow\n");
			}
			cp -= sizeof(double)-1;
			put_addr(store, cp, r);
			store_pc(cp);
			cp--;
			break;
		}
		//5
		//4
		//3
		//2
		//1
		//0
		case 125: {
			store_pc(op_code);
			break;
		}
		case 126: {
			int r = lexer->get<int>();
			store_pc(op_code);
			if (pc + sizeof(bool) > cp) {
				P5_ERR("Program code overflow\n");
			}
			store_pc(r == 1);
			break;
		}
		case 127: {
			lexer->skip_spaces();
			auto c = lexer->get<char>();
			if (c != '\'') {
				P5_ERR("expected \', got %c\n", c);
			}
			c = lexer->get<char>();
			store_pc(op_code);
			if (pc + sizeof(char) > cp) {
				P5_ERR("Program code overflow\n");
			}
			store_pc(c);
			c = lexer->get<char>();
			if (c != '\'') {
				P5_ERR("expected \', got %c\n", c);
			}
			break;
		}
		case 7: {
			lexer->skip_spaces();
			auto c = lexer->get<char>();
			if (c != '(') {
				P5_ERR("expected (, got %c\n", c);
			}
			c = ' ';
			std::set<P5::set_el_t> set;
			while (c != ')') {
				auto elem = lexer->get<P5::set_el_t>();
				set.insert(elem);
				c = lexer->get<char>();
			}
			if (pc > cp - P5::set_size) {
				P5_ERR("Constant table overflow\n");
			}
			
			cp -= P5::set_size-1;
			int i = 0;
			for (auto el: set) {
				put_addr(store, cp+i, el);
				i++;
			}
			while (i < P5::set_size) {
				P5::set_el_t el = 0;
				put_addr(store, cp+i, el);
				i++;
			}
			
			store_pc(op_code);
			store_pc(cp);
			
			cp--;
			break;
		}
		//chk
		case 26:
		case 95:
		case 96:
		case 97:
		case 98:
		case 99: {
			auto lb = lexer->get<int>();
			auto ub = lexer->get<int>();
			P5::addr_t q;
			if (op_code == 95) {
				q = lb;
			} else {
				if (pc > cp - 2*sizeof(lb)) {
					P5_ERR("Constant table overflow");
				}
				cp -= sizeof(lb)-1;
				put_addr(store, cp, lb);
				cp--;
				cp -= sizeof(ub)-1;
				put_addr(store, cp, ub);
				q = cp;
				cp--;
			}
			store_pc(op_code);
			store_pc(q);
			break;
		}
		//lca
		case 56: {
			printf("lca\n");
			auto l = lexer->get<int>();
			lexer->skip_spaces();
			auto c = lexer->get<char>();
			expect_char('\'', c);
			printf("lca params: %d %c\n", l, c);
			std::string str(l, ' ');
			c = lexer->get<char>();
			int i = 0;
			while (c != '\'') {
				str[i] = c;
				c = lexer->get<char>();
				i++;
			};
			printf("lca string: %s\n", str.c_str());
			if (pc > cp - l) {
				P5_ERR("Constant table overflow");
			}
			cp -= l-1;
			//TODO: include zero at the end?
			put_addr_by_ptr(store, cp, (unsigned char*)str.c_str(), str.size());
			P5::addr_t q = cp;
			cp--;
			store_pc(op_code);
			store_pc(q);
			break;
		}
		//ret
		case 14:
		case 128:
		case 129:
		case 130:
		case 131:
		case 132:
		//equ,neq,geq,grt,leq,les
		case 17:
		case 137:
		case 138:
		case 139:
		case 140:
		case 141:
		case 18:
		case 143:
		case 144:
		case 145:
		case 146:
		case 147:
		case 19:
		case 149:
		case 150:
		case 151:
		case 152:
		case 153:
		case 20:
		case 155:
		case 156:
		case 157:
		case 158:
		case 159:
		case 21:
		case 161:
		case 162:
		case 163:
		case 164:
		case 165:
		case 22:
		case 167:
		case 168:
		case 169:
		case 170:
		case 171:
		//ord
		case 59:
		case 133:
		case 134:
		case 135:
		case 136:
		//sto
		case 6:
		case 80:
		case 81:
		case 82:
		case 83:
		case 84:
		//eof,adi,adr,sbi,sbr,sgs,flt,flo,trc,ngi,ngr,sqi,sqr,abi,abr,not,and,
        //ior,dif,int,uni,inn,mod,odd,mpi,mpr,dvi,dvr,stp,chr,rnd,rgs,fbv, 
        //fvb
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
		case 58:
		case 60:
		case 62:
		case 110:
		case 111:
		case 115:
		case 116: {
			store_pc(op_code);
			break;
		}
		//ujc
		case 61: {
			store_pc(op_code);
			P5::addr_t q = 0;
			store_pc(q);
			break;
		}
		default: {
			P5_ERR("unsupported instruction: %d\n", op_code);
		}
	}
}

void Assembler::expect_char(char ec, char ac) {
	if (ec != ac) {
		P5_ERR("expected %c, got %c\n", ec, ac);
	}
}

template<typename T>
void Assembler::store_pc(T val) {
	if (pc > cp-sizeof(T)) {
		P5_ERR("Program code overflow\n");
	}
	put_addr(store, pc, val);
	pc += sizeof(T);
}

P5::addr_t Assembler::label_search() {
	char c;
	do {
		lexer->get<char>();
		c = lexer->peek();
	} while (c != 'l' && c != '\n');
	lexer->get<char>();
	auto x = lexer->get<int>();
	auto q = lb_table->lookup(x, pc);
	return q;
}

#define ins_op(name, opcode, has_p_p, q_len_p) instr[name] = opcode; op_codes[opcode] = {.has_p = (has_p_p), .q_len = (q_len_p)};
#define sp_table_op(name, opcode) sp_table[name] = opcode;

void Assembler::init() {
	ins_op("lodi", 0, true, P5::int_size)
	ins_op("ldoi", 1, false, P5::int_size)
	ins_op("stri", 2, true, P5::int_size)
	ins_op("sroi", 3, false, P5::int_size)
	ins_op("lda", 4, true, P5::int_size)
	ins_op("lao", 5, false, P5::int_size)
	ins_op("stoi", 6, false, 0)
	ins_op("ldc", 7, false, P5::int_size)
	ins_op("indi", 9, false, P5::int_size)
	ins_op("inci", 10, false, P5::int_size)
	ins_op("mst", 11, true, 0)
	ins_op("cup", 12, true, P5::int_size)
	ins_op("ents", 13, false, P5::int_size)
	ins_op("retp", 14, false, 0)
	ins_op("csp", 15, false, P5::int_size)
	ins_op("ixa", 16, false, P5::int_size)
	ins_op("equa", 17, false, 0)
	ins_op("neqa", 18, false, 0)
	ins_op("geqa", 19, false, 0)
	ins_op("grta", 20, false, 0)
	ins_op("leqa", 21, false, 0)
	ins_op("lesa", 22, false, 0)
	ins_op("ujp", 23, false, P5::int_size)
	ins_op("fjp", 24, false, P5::int_size)
	ins_op("xjp", 25, false, P5::int_size)
	ins_op("chki", 26, false, P5::int_size)
	ins_op("eof", 27, false, 0)
	ins_op("adi", 28, false, 0)
	ins_op("adr", 29, false, 0)
	ins_op("sbi", 30, false, 0)
	ins_op("sbr", 31, false, 0)
	ins_op("sgs", 32, false, 0)
	ins_op("flt", 33, false, 0)
	ins_op("flo", 34, false, 0)
	ins_op("trc", 35, false, 0)
	ins_op("ngi", 36, false, 0)
	ins_op("ngr", 37, false, 0)
	ins_op("sqi", 38, false, 0)
	ins_op("sqr", 39, false, 0)
	ins_op("abi", 40, false, 0)
	ins_op("abr", 41, false, 0)
	ins_op("not", 42, false, 0)
	ins_op("and", 43, false, 0)
	ins_op("ior", 44, false, 0)
	ins_op("dif", 45, false, 0)
	ins_op("int", 46, false, 0)
	ins_op("uni", 47, false, 0)
	ins_op("inn", 48, false, 0)
	ins_op("mod", 49, false, 0)
	ins_op("odd", 50, false, 0)
	ins_op("mpi", 51, false, 0)
	ins_op("mpr", 52, false, 0)
	ins_op("dvi", 53, false, 0)
	ins_op("dvr", 54, false, 0)
	ins_op("mov", 55, false, P5::int_size)
	ins_op("lca", 56, false, P5::int_size)
	ins_op("deci", 57, false, P5::int_size)
	ins_op("stp", 58, false, 0)
	ins_op("ordi", 59, false, 0)
	ins_op("chr", 60, false, 0)
	ins_op("ujc", 61, false, P5::int_size)
	ins_op("rnd", 62, false, 0)
	ins_op("pck", 63, false, P5::int_size)
	ins_op("upk", 64, false, P5::int_size)
	ins_op("ldoa", 65, false, P5::int_size)
	ins_op("ldor", 66, false, P5::int_size)
	ins_op("ldos", 67, false, P5::int_size)
	ins_op("ldob", 68, false, P5::int_size)
	ins_op("ldoc", 69, false, P5::int_size)
	ins_op("stra", 70, true, P5::int_size)
	ins_op("strr", 71, true, P5::int_size)
	ins_op("strs", 72, true, P5::int_size)
	ins_op("strb", 73, true, P5::int_size)
	ins_op("strc", 74, true, P5::int_size)
	ins_op("sroa", 75, false, P5::int_size)
	ins_op("sror", 76, false, P5::int_size)
	ins_op("sros", 77, false, P5::int_size)
	ins_op("srob", 78, false, P5::int_size)
	ins_op("sroc", 79, false, P5::int_size)
	ins_op("stoa", 80, false, 0)
	ins_op("stor", 81, false, 0)
	ins_op("stos", 82, false, 0)
	ins_op("stob", 83, false, 0)
	ins_op("stoc", 84, false, 0)
	ins_op("inda", 85, false, P5::int_size)
	ins_op("indr", 86, false, P5::int_size)
	ins_op("inds", 87, false, P5::int_size)
	ins_op("indb", 88, false, P5::int_size)
	ins_op("indc", 89, false, P5::int_size)
	ins_op("inca", 90, false, P5::int_size)
	ins_op("incr", 91, false, P5::int_size)
	ins_op("incs", 92, false, P5::int_size)
	ins_op("incb", 93, false, P5::int_size)
	ins_op("incc", 94, false, P5::int_size)
	ins_op("chka", 95, false, P5::int_size)
	ins_op("chkr", 96, false, P5::int_size)
	ins_op("chks", 97, false, P5::int_size)
	ins_op("chkb", 98, false, P5::int_size)
	ins_op("chkc", 99, false, P5::int_size)
	ins_op("deca", 100, false, P5::int_size)
	ins_op("decr", 101, false, P5::int_size)
	ins_op("decs", 102, false, P5::int_size)
	ins_op("decb", 103, false, P5::int_size)
	ins_op("decc", 104, false, P5::int_size)
	ins_op("loda", 105, true, P5::int_size)
	ins_op("lodr", 106, true, P5::int_size)
	ins_op("lods", 107, true, P5::int_size)
	ins_op("lodb", 108, true, P5::int_size)
	ins_op("lodc", 109, true, P5::int_size)
	ins_op("rgs", 110, false, 0)
	ins_op("fbv", 111, false, 0)
	ins_op("ipj", 112, true, P5::int_size)
	ins_op("cip", 113, true, 0)
	ins_op("lpa", 114, true, P5::int_size)
	ins_op("efb", 115, false, 0)
	ins_op("fvb", 116, false, 0)
	ins_op("dmp", 117, false, P5::int_size)
	ins_op("swp", 118, false, P5::int_size)
	ins_op("tjp", 119, false, P5::int_size)
	ins_op("lip", 120, true, P5::int_size)
	ins_op("ldci", 123, false, P5::int_size)
	ins_op("ldcr", 124, false, P5::int_size)
	ins_op("ldcn", 125, false, 0)
	ins_op("ldcb", 126, false, P5::bool_size)
	ins_op("ldcc", 127, false, P5::char_size)
	ins_op("reti", 128, false, 0)
	ins_op("retr", 129, false, 0)
	ins_op("retc", 130, false, 0)
	ins_op("retb", 131, false, 0)
	ins_op("reta", 132, false, 0)
	ins_op("ordr", 133, false, 0)
	ins_op("ordb", 134, false, 0)
	ins_op("ords", 135, false, 0)
	ins_op("ordc", 136, false, 0)
	ins_op("equi", 137, false, 0)
	ins_op("equr", 138, false, 0)
	ins_op("equb", 139, false, 0)
	ins_op("equs", 140, false, 0)
	ins_op("equc", 141, false, 0)
	ins_op("equm", 142, false, P5::int_size)
	ins_op("neqi", 143, false, 0)
	ins_op("neqr", 144, false, 0)
	ins_op("neqb", 145, false, 0)
	ins_op("neqs", 146, false, 0)
	ins_op("neqc", 147, false, 0)
	ins_op("neqm", 148, false, P5::int_size)
	ins_op("geqi", 149, false, 0)
	ins_op("geqr", 150, false, 0)
	ins_op("geqb", 151, false, 0)
	ins_op("geqs", 152, false, 0)
	ins_op("geqc", 153, false, 0)
	ins_op("geqm", 154, false, P5::int_size)
	ins_op("grti", 155, false, 0)
	ins_op("grtr", 156, false, 0)
	ins_op("grtb", 157, false, 0)
	ins_op("grts", 158, false, 0)
	ins_op("grtc", 159, false, 0)
	ins_op("grtm", 160, false, P5::int_size)
	ins_op("leqi", 161, false, 0)
	ins_op("leqr", 162, false, 0)
	ins_op("leqb", 163, false, 0)
	ins_op("leqs", 164, false, 0)
	ins_op("leqc", 165, false, 0)
	ins_op("leqm", 166, false, P5::int_size)
	ins_op("lesi", 167, false, 0)
	ins_op("lesr", 168, false, 0)
	ins_op("lesb", 169, false, 0)
	ins_op("less", 170, false, 0)
	ins_op("lesc", 171, false, 0)
	ins_op("lesm", 172, false, P5::int_size)
	ins_op("ente", 173, false, P5::int_size)
	ins_op("mrkl", 174, false, P5::int_size)


	sp_table_op("get", 0)
	sp_table_op("put", 1)
	sp_table_op("rln", 3)
	sp_table_op("new", 4)
	sp_table_op("wln", 5)
	sp_table_op("wrs", 6)
	sp_table_op("eln", 7)
	sp_table_op("wri", 8)
	sp_table_op("wrr", 9)
	sp_table_op("wrc", 10)
	sp_table_op("rdi", 11)
	sp_table_op("rdr", 12)
	sp_table_op("rdc", 13)
	sp_table_op("sin", 14)
	sp_table_op("cos", 15)
	sp_table_op("exp", 16)
	sp_table_op("log", 17)
	sp_table_op("sqt", 18)
	sp_table_op("atn", 19)
	sp_table_op("pag", 21)
	sp_table_op("rsf", 22)
	sp_table_op("rwf", 23)
	sp_table_op("wrb", 24)
	sp_table_op("wrf", 25)
	sp_table_op("dsp", 26)
	sp_table_op("wbf", 27)
	sp_table_op("wbi", 28)
	sp_table_op("wbr", 29)
	sp_table_op("wbc", 30)
	sp_table_op("wbb", 31)
	sp_table_op("rbf", 32)
	sp_table_op("rsb", 33)
	sp_table_op("rwb", 34)
	sp_table_op("gbf", 35)
	sp_table_op("pbf", 36)

}
