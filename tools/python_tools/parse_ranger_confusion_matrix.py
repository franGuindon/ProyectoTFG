import sys
  
def main():
  if len(sys.argv) != 2:
    print("Usage parse_ranger_confusion_matrix [RANGER_CONFUSION_MATRIX_FILE]")
    exit(1)

  with open(sys.argv[1]) as f: data = f.read()
  lines = data.split("\n")
  parsed_lines = [line.split() for line in lines]

  tp = int(parsed_lines[-2][-1])
  fp = int(parsed_lines[-2][-2])
  fn = int(parsed_lines[-3][-1])
  tn = int(parsed_lines[-3][-2])
  precision = tp/(tp+fp)
  recall = tp/(tp+fn)

  print(f"Precision: {precision:0.2f}")
  print(f"Recall:    {recall:0.2f}")
  
if __name__ == "__main__":
  main()
