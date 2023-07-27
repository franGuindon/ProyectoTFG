import numpy as np

from sklearn.model_selection import train_test_split
from skranger.ensemble import RangerForestClassifier

from utilities import read_float32_frame

# X = read_float32_frame()
# y = 

# X_train, X_test, y_train, y_test = train_test_split(X, y)

rfc = RangerForestClassifier()
# rfc.fit(X_train, y_train)

# predictions = rfc.predict(X_test)
# print(predictions)
# [1 2 0 0 0 0 1 2 1 1 2 2 2 1 1 0 1 1 0 1 1 1 0 2 1 0 0 1 2 2 0 1 2 2 0 2 0 0]

# probabilities = rfc.predict_proba(X_test)
# print(probabilities)
# [[0.01333333 0.98666667 0.        ]
#  [0.         0.         1.        ]
#  ...
#  [0.98746032 0.01253968 0.        ]
#  [0.99       0.01       0.        ]]