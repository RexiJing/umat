
#include<iostream>
#include<algorithm>
#include<cstring>
#include<cmath>
#include <utility>

using namespace std;

//定义常量
const double zero = 0.00, one = 1.00 , two=2.00, three=3.00 , four=4.00 , six=6.00 , enumax=0.499 , toler=1.0e-6;
const int newton=10,ntens=6,ndi=3,nprops=14,nvalue=7;
double table[2][8];
double Q[6][6];

//求平方
double pf(double x) 
{
	return x * x;
}

//定义硬化函数 AHARD
pair<double, double> AHARD(double syield, double hard, double eqplas, double (*table)[8], int nvalue)
{
	syield = table[0][7];
	hard = zero;
	cout << endl << "  eqplas=  " << eqplas << endl;
	pair<double, double> result;
	if (nvalue >= 1)
	{
		for (int i = 0; i < nvalue -1; i++)
		{
			double eqpl1 = table[1][1 + i];
			if (eqplas <= eqpl1)
			{
				double eqpl0 = table[1][i];
				if (eqpl1 <= eqpl0)
				{
					cout << "AHARD函数调用错误，退出计算" << endl;
					cout << eqpl1 << "  " << eqpl0 << "  " << eqplas << "  " <<i<< endl;
					break;
				}
				//计算hard
				double deqpl = eqpl1 - eqpl0;
				double syiel0 = table[0][i];
				double syiel1 = table[0][i + 1];
				double dsyiel = syiel1 - syiel0;
				hard = dsyiel / deqpl;
				syield = syiel0 + (eqplas - eqpl0) * hard;
				cout << "  eqpl1=" << eqpl1 << "  eqpl0=" << eqpl0 << "  syiel1=" << syiel1 << "  syiel0=" << syiel0 << endl;
				break;
			}

		}
		result.first = hard;
		result.second= syield;
	}
	else 
	{
		cout << "硬化函数输入数据不足！";
	}
	cout << "hard=" << hard << endl;
	return result;
}

//定义矩阵求逆函数brinv
void brinv(int  N)
{
	double t, d;
	int l = 1;
	int is[6], js[6];
	memset(is, 0, sizeof is);
	memset(js, 0, sizeof js);
	for (int k = 0; k < N; k++)
	{
		d = 0.0;
		for (int i = k; i < N; i++)
		{
			for (int j = k; j < N; j++)
			{
				if (abs(Q[i][j]) >= d)
				{
					d = abs(Q[i][j]);
					is[k] = i;
					js[k] = j;
				}
			}
		}
		if (d == 0.0)
		{
			l = 0;
			//return;
		}
		for (int j = 0; j < N; j++)
		{
			t = Q[k][j];
			Q[k][j] = Q[is[k]][j];
			Q[is[k]][j] = t;
		}
		for (int i = 0; i < N; i++)
		{
			t = Q[i][k];
			Q[i][k] = Q[i][js[k]];
			Q[i][js[k]] = t;
		}
		Q[k][k] = one / Q[k][k];
		for (int j = 0; j < N; j++)
		{
			if (j != k)
			{
				Q[k][j] = Q[k][j] * Q[k][k];
			}
		}
		for (int i = 0; i < N; i++)
		{
			if (i != k)
			{
				for (int j = 0; j < N; j++)
				{
					if (j != k)
					{
						Q[i][j] = Q[i][j] - Q[i][k] * Q[k][j];
					}
				}
			}
		}
		for (int i = 0; i < N; i++)
		{
			if (i != k)
			{
				Q[i][k] = -Q[i][k] * Q[k][k];
			}
		}
	}
	for (int k = N - 1; k >= 0; k--)
	{
		for (int j = 0; j < N; j++)
		{
			t = Q[k][j];
			Q[k][j] = Q[js[k]][j];
			Q[js[k]][j] = t;
		}
		for (int i = 0; i < N; i++)
		{
			t = Q[i][k];
			Q[i][k] = Q[i][is[k]];
			Q[i][is[k]] = t;
		}
	}
	return;
}



int main()
{
	int  NEWTON, nvalue;
	double nblfz, nblfm, nblfm1, nblfm2, jcobfm, invar1, invar2, smises, afa, yieldd, syiel0, fb, fc, ddfddj, B, dnbl, hard, const1, const2, temp1, temp2, temp3, temp4, cnbl;
	double ddsdde[ntens][ntens], jcobfz[ntens][ntens], ddbdds[ntens][ntens], ddeddb[ntens][ntens], R[ntens][ntens];
	double dstres[ntens], dstran[ntens], pstres[ntens], stress[ntens], stran[ntens], ddqds[ntens], atrt[ntens], rb[ntens], nblfmt[ntens], ddfdds[ntens], ddqdds[ntens], devias[ntens], R0[ntens], nblqr[ntens];
	//录入初始输入数组
	cout << "输入初始输入数组 props[20]，按顺序输入弹性模量、泊松比、摩擦角、膨胀角度、硬化数据（屈服应力-等效塑性应变）。"<<endl;
	//double props[20] = { 6400,0.23,35,35, 200,0,300,0.007,350,0.02,400,0.039,450,0.05,500,0.065,550,0.09,590,0.135 };
	double props[20];
    for (int i = 0; i < 20; i++)  cin >> props[i];
    //录入上一迭代步的状态数据
	cout << "输入上一步计算所得的状态数组 statev[13]，按顺序输入上一步弹性应变矩阵 EELAS[6]，塑性应变矩阵EPLAS[6]，等效塑性应变 EQPLAS。" << endl;
	//double statev[ntens * 2 + 1] = { 0.020621783,-0.056803634,0.016773326,-0.0034790024,-0.000098747095,0.0022516457,0.0050475128,-0.0061290882,0.0045365569,-0.00036778120,-0.000013610089,0.00028942950,0.0051668049 };
	double statev[ntens * 2 + 1];
	for (int i = 0; i <= ntens * 2; i++)  cin >> statev[i];
	//计算props状态数组，录入table矩阵
	nvalue = 8;
	double table[2][8];
	for (int i = 0; i < nvalue; i++)
	{
		table[0][i] = props[4 + 2 * i];
		table[1][i] = props[5 + 2 * i];
	}

	NEWTON = 10;
	nblfz = zero;
	nblfm1 = zero;
	nblfm2 = zero;
	hard = zero;
	dnbl = zero;
	jcobfm = zero;
	double emod = props[1];
	double enu = max(enumax, props[2]);
	double beta = props[3];
	double pusi = props[4];
	
	//计算3*K
	double ebulk3 = emod / (one-two*enu);
	//计算2*G
	double eg2 = emod / (one+enu);
	double eg = eg2 / 2;
	double eg3 = eg * three;
	double elam = (ebulk3-eg2) / three;


	//录入上一步计算后的应力、应变
	cout << "输入应力矩阵 stress[6]"<<endl;
	for (int i = 0; i < ntens; i++)  cin >> stress[i];
	cout << "输入应变矩阵 stran[6]" << endl;
	for (int i = 0; i < ntens; i++)  cin >> stran[i];

	//录入上一步计算后的应力、应变增量
	//cout << "输入应力增量矩阵 dstres[6]" << endl;
	//for (int i = 0; i < ntens; i++)  cin >> dstres[i];
	cout << "输入应变增量矩阵 dstran[6]" << endl;
	for (int i = 0; i < ntens; i++)  cin >> dstran[i];




	//初始化矩阵
	for (int i = 0; i < ntens; i++)
	{
		for (int j = 0; j < ntens; j++)
		{
			ddsdde[i][j] = zero;
			jcobfz[i][j] = zero;
		}
		dstres[i] = zero;
		ddqds[i] = zero;
		atrt[i] = zero;
		rb[i] = zero;
		nblfmt[i] = zero;
	}
	syiel0 = props[4];

	//弹性设置
	for (int i = 0; i < ndi; i++)
	{
		for (int j = 0; j < ndi; j++)
		{
			ddsdde[j][i] = elam;
		}
	}
	for (int i = 0; i < ndi; i++)
	{
		for (int j = 0; j < ndi; j++)
		{
			ddsdde[i][i] = eg2 + elam;
		}
	}
	for (int i = 3; i < ntens; i++)
	{
		ddsdde[i][i] = eg;
	}
	//调用rotsig函数

	//读取弹塑性应变、等效塑性应变
	double eelas[ntens], eplas[ntens];
	double eqplas;
	for (int i = 0; i < ntens; i++)
	{
		eelas[i] = statev[i];
		eplas[i] = statev[ntens+i];
	}
	eqplas= statev[ntens*2];

	//计算弹性试应力
	for (int i = 0; i < ntens; i++)
	{
		for (int j = 0; j < ntens; j++)
		{
			dstres[j] = dstres[j] + ddsdde[j][i] * dstran[i];
		}
	}
	for (int i = 0; i < ntens; i++)
	{
		eelas[i] = eelas[i] + dstran[i];
	}
	for (int i = 0; i < ntens; i++)
	{
		pstres[i] = dstres[i] + stress[i];
	}
	cout << endl << "输出B点应力向量" << endl;
	for (int i = 0; i < ntens; i++)
	{
		cout << pstres[i] << "  ";
	}
	cout << endl;
	//计算应力不变量
	invar1 = pstres[0] + pstres[1] + pstres[2];
	smises = pf(pstres[0] - pstres[1]) + pf(pstres[0] - pstres[2]) + pf(pstres[1] - pstres[2]);
	for (int i = 3; i < ntens; i++)
	{
		smises = smises + six * pf(pstres[i]);
	}
	invar2 = smises / six;
	cout << endl << "输出B点应力不变量" << endl;
	cout << "invar1=  "<<invar1<<"  invar2=  "<<invar2<<endl;

	//计算B点屈服函数
	afa = tan(beta) / (three * sqrt(three));
	yieldd = (1 - tan(beta) / three) * syiel0 / sqrt(three);
	fb = sqrt(invar2) + afa * invar1 - yieldd;

	//输出B点应力
	cout << endl << "B点应力" << endl;
	for (int i = 0; i < ntens; i++)
	{
		cout << pstres[i] << "  ";
	}
	cout << endl;
	//输出B点屈服函数
	cout << "fb=" << fb << endl;

	//判断进入弹性or塑性，并分别计算
	if (fb > toler)//进入塑性
	{
		//塑性阶段，对应fortran——testgm1  line109
		cout << "进入塑性阶段" << endl;
		//调用ahard函数
		cout << "第一次调用 AHARD函数" << endl;
		nvalue = 8;
		pair<double, double> result = AHARD(syiel0, hard, eqplas, table, nvalue);
		hard = result.first;
		double syield = result.second;
		//计算B点DDFDDS
		ddfddj = sqrt(one / (four * invar2));
		for (int i = 0; i < ndi; i++)
		{
			ddfdds[i] = afa + ddfddj * (pstres[i] - invar1 / three);
		}
		for (int i = 3; i < ntens; i++)
		{
			ddfdds[i] = ddfddj * pstres[i] * two;
		}
		cout << endl << "输出B点DDFDDS" << endl;
		for (int i = 0; i < ntens; i++)
		{
			cout << ddfdds[i] << "  ";
		}
		cout << endl;
		//计算DDQDDS
		for (int i = 0; i < ndi; i++)
		{
			ddqdds[i] = (one / three) * tan(pusi) + ddfddj * (pstres[i] - invar1 / three);
		}
		for (int i = 3; i < ntens; i++)
		{
			ddqdds[i] = ddfddj * two * pstres[i];
		}
		cout << endl << "输出B点DDQDDS" << endl;
		for (int i = 0; i < ntens; i++)
		{
			cout << ddqdds[i] << "  ";
		}
		cout << endl;
		//计算D*DDQDDS=DDQDS
		for (int i = 0; i < ntens; i++)
		{
			for (int j = 0; j < ntens; j++)
			{
				ddqds[j] = ddqds[j] + ddsdde[j][i] * ddqdds[i];
			}
		}
		cout << endl << "输出B点DDQDS" << endl;
		for (int i = 0; i < ntens; i++)
		{
			cout << ddqds[i] << "  ";
		}
		cout << endl;
		//计算NBLFM2
		B = pf(ddqdds[1]- ddqdds[2])+ pf(ddqdds[3] - ddqdds[2])+ pf(ddqdds[1] - ddqdds[3]);
		for (int i = 3; i < ntens; i++)
		{
			B += six * pf(ddqdds[i]);
		}
		B /= six;
		B = sqrt(two * B / three);
		nblfm2 = (one - afa) * hard * B;
		cout << endl << "输出B点nblfm2,B" << endl;
		cout << nblfm2 << "  " << B << endl;

		//计算NBLFM1
		for (int i = 0; i < ntens; i++)
		{
			nblfm1 += ddfdds[i] * ddqds[i];
		}
		cout << endl << "输出B点nblfm1" << endl;
		cout << nblfm1 << endl;
		//计算DNBL
		dnbl = fb / (nblfm1 + nblfm2);
		cout << endl << "计算C点应力前的dnbl=  " << dnbl << endl;
		//计算C点应力
		for (int i = 0; i < ntens; i++)
		{
			stress[i] = pstres[i] - dnbl * ddqds[i];
		}
		//输出C点应力矩阵
		cout << endl << "C点应力" << endl;
		for (int i = 0; i < ntens; i++)
		{
			cout<<stress[i]<<"  ";
		}







		cout << endl << "进行newton迭代" << endl;
		//进行newton迭代计算
		for (int knewton = 0; knewton < NEWTON; knewton++)
		{
			//计算C点应力不变量
			invar1 = stress[0] + stress[1] + stress[2];
			smises = pf(stress[0] - stress[1]) + pf(stress[0] - stress[2]) + pf(stress[1] - stress[2]);
			for (int i = 3; i < ntens; i++)
			{
				smises += six * pf(stress[i]);
			}
			invar2 = smises / six;
			cout << endl << "输出C点应力不变量" << endl;
			cout << invar1 << "  " << invar2 << endl;
			//计算偏应力
			for (int i = 0; i < ndi; i++)
			{
				devias[i] = stress[i] - invar1 / three;
			}
			//计算DDFDDS
			ddfddj = sqrt(one / (four * invar2));
			for (int i = 0; i < ndi; i++)
			{
				ddfdds[i] = afa + ddfddj * devias[i];
			}
			for (int i = 3; i < ntens; i++)
			{
				ddfdds[i] = ddfddj * stress[i] * two;
			}
			cout << endl << "输出C点DDFDDS" << endl;
			for (int i = 0; i < ntens; i++)
			{
				cout << ddfdds[i] << "  ";
			}
			cout << endl;
			//计算DDQDDS
			for (int i = 0; i < ndi; i++)
			{
				ddqdds[i] = (one / three) * tan(pusi) + ddfddj * devias[i];
			}
			for (int i = 3; i < ntens; i++)
			{
				ddqdds[i] = ddfddj * two * stress[i];
			}
			cout << endl << "输出C点DDQDDS" << endl;
			for (int i = 0; i < ntens; i++)
			{
				cout << ddqdds[i] << "  ";
			}
			cout << endl;
			//计算C点DDQDS
			for (int i = 0; i < ntens; i++)
			{
				ddqds[i] = zero;
			}
			for (int i = 0; i < ntens; i++)
			{
				for (int j = 0; j < ntens; j++)
				{
					ddqds[j] = ddqds[j] + ddsdde[j][i] * ddqdds[i];
				}
			}
			cout << endl << " C点DDQDS " << endl;
			for (int i = 0; i < ntens; i++)
			{
				cout <<"  " << ddqds[i] << "  ";
			}
			cout << endl;

			//计算屈服面及参数
			B = pf(ddqdds[1] - ddqdds[2]) + pf(ddqdds[2] - ddqdds[3]) + pf(ddqdds[1] - ddqdds[3]);
			for (int i = 3; i < ntens; i++)
			{
				B += six * pf(ddqdds[i]);
			}
			B /= six;
			B = sqrt(two * B / three);
			eqplas = statev[12] + dnbl * B;

			cout << endl << "  statev[12]=  " << statev[12] << "  dnbl=   " << dnbl  << "  B=   " << B << endl;
			cout << " eqplas= " << eqplas << endl;

			//调用ahard函数
			cout << "第二次调用 AHARD函数" << endl;
			pair<double, double> result = AHARD(syiel0, hard, eqplas, table, nvalue);
			hard = result.first;
			double syield = result.second;

			//计算C点屈服函数
			nblfm2 = (one - afa) * hard * B;
			yieldd = (1 - tan(beta) / three) * syiel0 / sqrt(three);
			fc = sqrt(invar2) + invar1 * afa - yieldd;
			
			cout << "fc=" << fc << endl;

			//计算C点DDBDDS
			const1 = sqrt(three) / two;
			const2 = one / sqrt(invar2);
			temp1 = -one * const2 * const2 * const2 / two;
			temp2 = two * const2 / three;
			temp3 = one * const2 / three;
			temp4 = two * const2;
			for (int i = 0; i < ndi; i++)
			{
				ddbdds[i][i] = const1 * (temp1 * pf(devias[i]) + temp2) / sqrt(three);
			}
			for (int i = 3; i < ntens; i++)
			{
				ddbdds[i][i] = const1 * (temp1 * four * pf(stress[i]) + temp4) / sqrt(three);
			}
			ddbdds[0][1] = const1 * (temp1 * devias[0] * devias[1] + temp3) / sqrt(three);
			ddbdds[0][2] = const1 * (temp1 * devias[0] * devias[2] + temp3) / sqrt(three);
			ddbdds[1][2] = const1 * (temp1 * devias[1] * devias[2] + temp3) / sqrt(three);

			ddbdds[0][3] = temp1 * devias[0] * stress[3];
			ddbdds[0][4] = temp1 * devias[0] * stress[4];
			ddbdds[0][5] = temp1 * devias[0] * stress[5];
			ddbdds[1][3] = temp1 * devias[1] * stress[3];
			ddbdds[1][4] = temp1 * devias[1] * stress[4];
			ddbdds[1][5] = temp1 * devias[1] * stress[5];
			ddbdds[2][3] = temp1 * devias[2] * stress[3];
			ddbdds[2][4] = temp1 * devias[2] * stress[4];
			ddbdds[2][5] = temp1 * devias[2] * stress[5];

			ddbdds[3][4] = temp1 * two * stress[3] * stress[4];
			ddbdds[3][5] = temp1 * two * stress[3] * stress[5];
			ddbdds[4][5] = temp1 * two * stress[4] * stress[5];

			for (int i = 0; i < ntens; i++)
			{
				for (int j = 0; j < ntens; j++)
				{
					if (i > j)  ddbdds[i][j] = ddbdds[j][i];
				}
			}
			//计算C点DDEDDB   DDSDDE*DDBDDS=DDEDDB
			//line 258
			for (int i = 0; i < ntens; i++)
			{
				for (int j = 0; j < ntens; j++)
				{
					ddeddb[i][j] = zero;
				}
			}
			for (int i = 0; i < ntens; i++)
			{
				for (int j = 0; j < ntens; j++)
				{
					for (int k = 0; k < ntens; k++)
					{
						ddeddb[i][j] += ddsdde[i][k] * ddbdds[k][j];
					}
				}
			}
			//计算Q矩阵
			for (int i = 0; i < ntens; i++)
			{
				for (int j = 0; j < ntens; j++)
				{
					Q[i][j] = dnbl * ddeddb[i][j];
				}
			}
			for (int i = 0; i < ntens; i++)
			{
				Q[i][i] += one;
			}
			//调用brinv函数求矩阵逆
			brinv(6);

			if (abs(fc) <= toler)
			{
				break;
			}
			else
			{
				//展开下一步迭代的校正
				//计算C点NBLFM2，上述步骤已算
				//计算NBLFM1，Q-1*DDQDS=NBLFMT
				for (int i = 0; i < ntens; i++)
				{
					nblfmt[i] = zero;
				}
				cout << endl;
				for (int i = 0; i < ntens; i++)
				{
					for (int j = 0; j < ntens; j++)
					{
						nblfmt[j] += Q[j][i] * ddqds[i];
					}
				}
				cout << endl << "输出C点NBLFMT" << endl;
				for (int i = 0; i < ntens; i++)
				{
					cout << nblfmt[i] << "  ";
				}
				nblfm1 = zero;
				for (int i = 0; i < ntens; i++)
				{
					nblfm1 += nblfmt[i] * ddfdds[i];
				}
				nblfm = nblfm1 + nblfm2;
				cout << endl << "C点NBLFM  " << nblfm << endl;
				//计算 NBLFZ,R0
				for (int i = 0; i < ntens; i++)
				{
					R0[i] = stress[i] - pstres[i] + dnbl * ddqds[i];
				}
				cout << endl << "输出C点R0" << endl;
				for (int i = 0; i < ntens; i++)
				{
					cout << R0[i] << "  ";
				}
				cout << endl;
				//计算NBLA,FENZI,NBLQR
				for (int i = 0; i < ntens; i++)
				{
					nblqr[i] = zero;
				}
				for (int i = 0; i < ntens; i++)
				{
					for (int j = 0; j < ntens; j++)
					{
						nblqr[j] += Q[j][i] * R0[i];
					}
				}
				cout << endl << "输出C点NBLQR" << endl;
				for (int i = 0; i < ntens; i++)
				{
					cout << nblqr[i] << "  ";
				}
				cout << endl;
				nblfz = zero;
				for (int i = 0; i < ntens; i++)
				{
					nblfz += ddfdds[i] * nblqr[i];
				}
				nblfz = fc - nblfz;
				//计算CNBL，修正NABLA
				cnbl = nblfz / nblfm;
				cout << endl << "  fz=  " << nblfz << "  fm=  " << nblfm << endl;
				cout << endl << " dnbl= " << dnbl << "  cnbl= " << cnbl << endl;
				//应力修正
				for (int i = 0; i < ntens; i++)
				{
					dstres[i] = -nblqr[i] - cnbl * nblfmt[i];
				}
				//更新C点的stress与DNBL
				for (int i = 0; i < ntens; i++)
				{
					stress[i] += dstres[i];
				}
				dnbl += cnbl;
				cout << endl << " dnbl= " << dnbl << "  cnbl= " << cnbl << endl;
				cout << "更新后C点应力向量" << endl;
				for (int i = 0; i < ntens; i++)
				{
					cout << stress[i] << " ";
				}
				cout << endl;
			}
			
		}

		//更新弹性、塑性应变
		for (int i = 0; i < ntens; i++)
		{
			eelas[i] -= dnbl * ddqdds[i];
			eplas[i] += dnbl * ddqdds[i];
		}
		//计算一致切线刚度矩阵jacobian
		//计算R
		for (int i = 0; i < ntens; i++)
		{
			for (int j = 0; j < ntens; j++)
			{
				R[i][j] = zero;
			}
		}
		for (int i = 0; i < ntens; i++)
		{
			for (int j = 0; j < ntens; j++)
			{
				for (int k = 0; k < ntens; k++)
				{
					R[i][j] += Q[i][k] * ddsdde[k][j];
				}
			}
		}
		//计算JCOBFZ
		for (int i = 0; i < ntens; i++)
		{
			for (int j = 0; j < ntens; j++)
			{
				atrt[j] += R[j][i] * ddfdds[i];
				rb[j]+= R[j][i] * ddqdds[i];
			}
		}
		for (int i = 0; i < ntens; i++)
		{
			for (int j = 0; j < ntens; j++)
			{
				jcobfz[i][j] = rb[i] * atrt[j];
			}
		}
		//计算JCOBFM
		for (int i = 0; i < ntens; i++)
		{
			jcobfm += ddfdds[i] * rb[i];
		}
		jcobfm += nblfm2;
		for (int i = 0; i < ntens; i++)
		{
			for (int j = 0; j < ntens; j++)
			{
				ddsdde[i][j] = R[i][j] - jcobfz[i][j] / jcobfm;
			}
		}
	}
    else
	{
		//弹性阶段，更新应力
		for (int i = 0; i < ntens; i++)
		{
			stress[i] = pstres[i];
		}
	}

	//存储弹塑性应变、等效塑性应变
	for (int i = 0; i < ntens; i++)
	{
		statev[i] = eelas[i];
		statev[ntens + i] = eplas[i];
	}
	statev[12] = eqplas;

	//输出本步计算结果
	//输出应力向量
	cout << "输出应力向量" << endl;
	for (int i = 0; i < ntens; i++)
	{
		cout << stress[i] << "  ";
	}
	cout << endl;
	//输出弹性应变向量
	cout << "输出弹性应变向量" << endl;
	for (int i = 0; i < ntens; i++)
	{
		cout << eelas[i] << "  ";
	}
	cout << endl;
	//输出塑性应变向量
	cout << "输出塑性应变向量" << endl;
	for (int i = 0; i < ntens; i++)
	{
		cout << eplas[i] << "  ";
	}
	cout << endl;
	//输出等效塑性应变
	cout << "输出等效塑性应变" << endl<< eqplas<<endl;
	//输出jacobian矩阵
	cout << "输出jacobian矩阵" << endl;
	for (int i = 0; i < ntens; i++)
	{
		for (int j = 0; j < ntens; j++)
		{
			cout << ddsdde[i][j] << "  ";
		}
		cout << endl;
	}
	cout << endl;
	return 0;
}
