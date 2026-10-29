
- 设A为n阶方阵，$\lambda_i$为其特征值,$m_i$和$\alpha_i$分别为其代数重数和几何重数($m_i$ >= $\alpha_i$).如果$m_i$=$\alpha_i$，则称特征值$\lambda_i$为半单的；如果$m_i$>$\alpha_i$，则称特征值$\lambda_i$为亏损的。并且：
	- 代数重数为一的特征值必定是半单的
	- 不同特征值对应的特征向量线性无关
	- *每个特征值都是半单的矩阵(具有完备特征向量系)等价于可以对角化*
	- *存在亏损特征值的矩阵称为亏损矩阵等价于不可对角化(可以化为约当标准型)*
---

设A为n阶方阵，则存在n阶可逆矩阵T使得
$$
A = TJT^{-1}$$
其中$$\boldsymbol{J}=\mathrm{diag}(\boldsymbol{J}_{n_1}(\lambda_1),\boldsymbol{J}_{n_2}(\lambda_2),\ldots,\boldsymbol{J}_{n_k}(\lambda_\mathrm{k})),n_1+n_2+\cdots n_k=n$$
$$\boldsymbol{J}_k(\lambda)=\begin{pmatrix}\lambda&1\\&\lambda&1\\&&\ddots&\ddots\\&&&\lambda&1\\&&&&\lambda\end{pmatrix}$$代表以$\lambda$为特征值的k阶(k x k)的约当块

称上式为A的Jordan分解，J称为A的Jordan标准型，T称为变换矩阵。*若不计Jordan块的次序，则Jordan标准型唯一*，很显然对于没有重根的时候，所有的jordan块都是一阶的，会变成对角阵。

- $\lambda_i$的代数重数就是以$\lambda_i$为特征值的约当块的阶数之和
- $\lambda_i$的几何重数就是以$\lambda_i$为特征值的约当块的个数之和

而具体的阶数分配则由下面的规则决定：
设A为n阶方阵，λ,为其特征值，则A的Jordan标准型J中以λ,为特征值，阶数为l的Jordan块的个数为
$$
r_{l+1}+r_{l-1}-2r_l$$
其中$r_l=\mathrm{rank}(\lambda_i\mathbf{I}-\mathbf{A})^l$  (注意是先求矩阵的n次方再求秩)

---
对于