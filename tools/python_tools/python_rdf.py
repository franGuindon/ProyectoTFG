import numpy as np

from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split

#X_train, X_test, y_train, y_test = train_test_split(df.drop('target', axis=1), df['target'], test_size=0.2, random_state=0)

CT = 1000
X1 = np.array([[1]*2]*CT)
X0 = np.array([[0]*2]*CT)
y1 = np.array([[1]]*CT)
y0 = np.array([[0]]*CT)
print("Dataset 1 features shape:", X1.shape)
print(X1[:3,:3])
X = np.array([X1,X0])
y = np.array([[X1],[X0]])

forest = RandomForestClassifier(n_estimators=100, random_state=0)
forest.fit(X,y)
print(f"{forest.score(X,y)}")
