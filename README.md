# Pollard's kangaroo for SECPK1

A simple Pollard's kangaroo ECDLP solver for SECPK1

Structure of the input file:
* All values are in hex format
* Public keys can be given either in compressed or uncompressed format

```
Start range
End range
Key #1
Key #2
...
```

ex

```
49dccfd96dc5df56487436f5a1b18c4f5d34f65ddb48cb5e0000000000000000
49dccfd96dc5df56487436f5a1b18c4f5d34f65ddb48cb5effffffffffffffff
0459A3BFDAD718C9D3FAC7C187F1139F0815AC5D923910D516E186AFDA28B221DC994327554CED887AAE5D211A2407CDD025CFC3779ECB9C9D7F2F1A1DDF3E9FF8
0335BB25364370D4DD14A9FC2B406D398C4B53C85BE58FCC7297BD34004602EBEC
```

# How it works

The program uses 2 herds of kangaroos, a tame herd and a wild herd. When 2 kangoroos (a wild one and a tame one) collide, the 
key can be solved. Due to the birthday paradox, a collision happens (in average) after 2.sqrt(k2-k1) group operations, the 2 
herds have the same size. Here is a brief description of the algoritm:

We have to solve P = k.G, we know that k lies in the range [k1,k2], G is the SecpK1 generator point.

n = floor(sqrt(k2-k1))+1

* Create a jump point table jP = [G,2G,4G,8G,...,2^nG], 
* Create a jump distance table jD = [1,2,4,8,....,2^n]
 
for all i in herdSize</br>
&nbsp;&nbsp;tame<sub>i</sub> = rand(0..(k2-k1))</br>
&nbsp;&nbsp;tamePos<sub>i</sub> = tame<sub>i</sub>.G  # Group operation</br>
&nbsp;&nbsp;wild<sub>i</sub> = rand(0..(k2-k1))</br>
&nbsp;&nbsp;wildPos<sub>i</sub> = P + wild<sub>i</sub>.G # Group operation</br>

found = false</br>

while not found</br>
&nbsp;&nbsp;for all i in herdSize</br>
&nbsp;&nbsp;&nbsp;&nbsp;  tamePos<sub>i</sub> = tamePos<sub>i</sub> + jP[tamePos<sub>i</sub>.x % n] # Group operation</br>
&nbsp;&nbsp;&nbsp;&nbsp;  tame<sub>i</sub> += jD[tamePos<sub>i</sub>.x % n]</br>
&nbsp;&nbsp;&nbsp;&nbsp;  wildPos<sub>i</sub> = wildPos<sub>i</sub> + jP[wildPos<sub>i</sub>.x % n] # Group operation</br>
&nbsp;&nbsp;&nbsp;&nbsp;  wild<sub>i</sub> += jD[wildPos<sub>i</sub>.x % n]</br>
&nbsp;&nbsp;&nbsp;&nbsp;  add tamePos<sub>i</sub> and wildPos<sub>i</sub> to hashTable</br>
&nbsp;&nbsp;found = is there a collision in hashTable between a tame and a wild kangaroo ?</br>
</br>

(t,w) = index of collision</br>
K = k1 + tame<sub>t</sub> - wild<sub>w</sub></br>

# Example of usage

On a Xeon X5647 2.93GHz with 12GB of RAM (Ubuntu 18.04)

Input file: in.txt

```
49dccfd96dc5df56487436f5a1b18c4f5d34f65ddb48cb5e0000000000000000
49dccfd96dc5df56487436f5a1b18c4f5d34f65ddb48cb5effffffffffffffff
0459A3BFDAD718C9D3FAC7C187F1139F0815AC5D923910D516E186AFDA28B221DC994327554CED887AAE5D211A2407CDD025CFC3779ECB9C9D7F2F1A1DDF3E9FF8
04A50FBBB20757CC0E9C41C49DD9DF261646EE7936272F3F68C740C9DA50D42BCD3E48440249D6BC78BC928AA52B1921E9690EBA823CBC7F3AF54B3707E6A73F34
0404A49211C0FE07C9F7C94695996F8826E09545375A3CF9677F2D780A3EB70DE3BD05357CAF8340CB041B1D46C5BB6B88CD9859A083B0804EF63D498B29D31DD1
040B39E3F26AF294502A5BE708BB87AEDD9F895868011E60C1D2ABFCA202CD7A4D1D18283AF49556CF33E1EA71A16B2D0E31EE7179D88BE7F6AA0A7C5498E5D97F
04837A31977A73A630C436E680915934A58B8C76EB9B57A42C3C717689BE8C0493E46726DE04352832790FD1C99D9DDC2EE8A96E50CAD4DCC3AF1BFB82D51F2494
040ECDB6359D41D2FD37628C718DDA9BE30E65801A88A00C3C5BDF36E7EE6ADBBAD71A2A535FCB54D56913E7F37D8103BA33ED6441D019D0922AC363FCC792C29A
0422DD52FCFA3A4384F0AFF199D019E481D335923D8C00BADAD42FFFC80AF8FCF038F139D652842243FC841E7C5B3E477D901F88C5AB0B88EE13D80080E413F2ED
04DB4F1B249406B8BD662F78CBA46F5E90E20FE27FC69D0FBAA2F06E6E50E536695DF83B68FD0F396BB9BFCF6D4FE312F32A43CF3FA1FE0F81DF70C877593B64E0
043BD0330D7381917F8860F1949ACBCCFDC7863422EEE2B6DB7EDD551850196687528B6D2BC0AA7A5855D168B26C6BAF9DDCD04B585D42C7B9913F60421716D37A
04332A02CA42C481EAADB7ADB97DF89033B23EA291FDA809BEA3CE5C3B73B20C49C410D1AD42A9247EB8FF217935C9E28411A08B325FBF28CC2AF8182CE2B5CE38
04513981849DE1A1327DEF34B51F5011C5070603CA22E6D868263CB7C908525F0C19EBA6BD2A8DCF651E4342512EDEACB6EA22DA323A194E25C6A1614ABD259BC0
04D4E6FA664BD75A508C0FF0ED6F2C52DA2ADD7C3F954D9C346D24318DBD2ECFC6805511F46262E10A25F252FD525AF1CBCC46016B6CD0A7705037364309198DA1
0456B468963752924DBF56112633DC57F07C512E3671A16CD7375C58469164599D1E04011D3E9004466C814B144A9BCB7E47D5BACA1B90DA0C4752603781BF5873
04D5BE7C653773CEE06A238020E953CFCD0F22BE2D045C6E5B4388A3F11B4586CBB4B177DFFD111F6A15A453009B568E95798B0227B60D8BEAC98AF671F31B0E2B
04B1985389D8AB680DEDD67BBA7CA781D1A9E6E5974AAD2E70518125BAD5783EB5355F46E927A030DB14CF8D3940C1BED7FB80624B32B349AB5A05226AF15A2228
0455B95BEF84A6045A505D015EF15E136E0A31CC2AA00FA4BCA62E5DF215EE981B3B4D6BCE33718DC6CF59F28B550648D7E8B2796AC36F25FF0C01F8BC42A16FD9
```

Result:

```
pons@linpons:~/Kangaroo$ ./kangaroo in.txt 
Kangaroo v1.0
Start:49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E0000000000000000
Stop :49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5EFFFFFFFFFFFFFFFF
Keys :16
Number of CPU thread: 8
Range width: 2^64
Number of random walk: 2^13.00 (Max DP=17)
DP size: 17 [0xffff800000000000]
Solvekey Thread 0: 1024 TAME kangaroos
Solvekey Thread 1: 1024 TAME kangaroos
Solvekey Thread 2: 1024 TAME kangaroos
Solvekey Thread 3: 1024 TAME kangaroos
Solvekey Thread 4: 1024 WILD kangaroos
Solvekey Thread 5: 1024 WILD kangaroos
Solvekey Thread 6: 1024 WILD kangaroos
Solvekey Thread 7: 1024 WILD kangaroos
[18.00 MKey/s][Count 2^34.40][20:47][Dead 10][20.2MB]  
Key# 0 Pub:  0x0259A3BFDAD718C9D3FAC7C187F1139F0815AC5D923910D516E186AFDA28B221DC 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5EBB3EF3883C1866D4 
[18.18 MKey/s][Count 2^33.80][13:44][Dead 3][15.4MB]  
Key# 1 Pub:  0x02A50FBBB20757CC0E9C41C49DD9DF261646EE7936272F3F68C740C9DA50D42BCD 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5EB5ABC43BEBAD3207 
[18.07 MKey/s][Count 2^30.28][01:12][Dead 1][2.8MB]  
Key# 2 Pub:  0x0304A49211C0FE07C9F7C94695996F8826E09545375A3CF9677F2D780A3EB70DE3 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E5698AAAB6CAC52B3 
[18.18 MKey/s][Count 2^32.78][06:44][Dead 2][9.4MB]  
Key# 3 Pub:  0x030B39E3F26AF294502A5BE708BB87AEDD9F895868011E60C1D2ABFCA202CD7A4D 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E59C839258C2AD7A0 
[18.18 MKey/s][Count 2^33.82][13:54][Dead 3][15.6MB]  
Key# 4 Pub:  0x02837A31977A73A630C436E680915934A58B8C76EB9B57A42C3C717689BE8C0493 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E765FB411E63B92B9 
[18.18 MKey/s][Count 2^33.07][08:16][Dead 1][10.9MB]  
Key# 5 Pub:  0x020ECDB6359D41D2FD37628C718DDA9BE30E65801A88A00C3C5BDF36E7EE6ADBBA 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E7D0E6081C7E0E865 
[18.18 MKey/s][Count 2^34.45][21:27][Dead 4][20.7MB]  
Key# 6 Pub:  0x0322DD52FCFA3A4384F0AFF199D019E481D335923D8C00BADAD42FFFC80AF8FCF0 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5EC737344CA673CE28 
[18.18 MKey/s][Count 2^33.50][11:06][Dead 0][13.4MB]  
Key# 7 Pub:  0x02DB4F1B249406B8BD662F78CBA46F5E90E20FE27FC69D0FBAA2F06E6E50E53669 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E38160DA9EBEAECD7 
[18.17 MKey/s][Count 2^33.20][09:02][Dead 1][11.6MB]  
Key# 8 Pub:  0x023BD0330D7381917F8860F1949ACBCCFDC7863422EEE2B6DB7EDD551850196687 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E79D808CAB1DECF8D 
[18.18 MKey/s][Count 2^32.25][04:40][Dead 0][7.2MB]  
Key# 9 Pub:  0x02332A02CA42C481EAADB7ADB97DF89033B23EA291FDA809BEA3CE5C3B73B20C49 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E54CAD3CFBC2A9C2B 
[18.18 MKey/s][Count 2^32.75][06:38][Dead 1][9.3MB]  
Key#10 Pub:  0x02513981849DE1A1327DEF34B51F5011C5070603CA22E6D868263CB7C908525F0C 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E0D5ECCC38D0230E6 
[18.18 MKey/s][Count 2^33.63][12:12][Dead 1][14.3MB]  
Key#11 Pub:  0x03D4E6FA664BD75A508C0FF0ED6F2C52DA2ADD7C3F954D9C346D24318DBD2ECFC6 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5EE3579364DE939B0C 
[18.18 MKey/s][Count 2^33.51][11:12][Dead 0][13.5MB]  
Key#12 Pub:  0x0356B468963752924DBF56112633DC57F07C512E3671A16CD7375C58469164599D 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E7C43B8E079AE7278 
[18.16 MKey/s][Count 2^33.89][14:33][Dead 3][16.1MB]  
Key#13 Pub:  0x03D5BE7C653773CEE06A238020E953CFCD0F22BE2D045C6E5B4388A3F11B4586CB 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E8D63EF128EF66B42 
[18.18 MKey/s][Count 2^34.10][16:51][Dead 5][17.7MB]  
Key#14 Pub:  0x02B1985389D8AB680DEDD67BBA7CA781D1A9E6E5974AAD2E70518125BAD5783EB5 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E2452DD26BC983CD5 
[18.18 MKey/s][Count 2^33.92][14:55][Dead 3][16.3MB]  
Key#15 Pub:  0x0355B95BEF84A6045A505D015EF15E136E0A31CC2AA00FA4BCA62E5DF215EE981B 
       Priv: 0x49DCCFD96DC5DF56487436F5A1B18C4F5D34F65DDB48CB5E7AD38337C7F173C7 

Done: Total time 03:07:38 
```
