#-------------------------------------------------------------------------------
# Imports
import sys

#-------------------------------------------------------------------------------
# Arg Parse
if 2 != len(sys.argv):
  print("Usage: parse_readme_into_list.py [SNIP_SPEC]")
  exit(1)

#-------------------------------------------------------------------------------
# Constants
spec_beg_specifier = "<!-- BEGIN SNIP SPEC -->"
spec_end_specifier = "<!-- END SNIP SPEC -->"
snip_spec = sys.argv[1]

#-------------------------------------------------------------------------------
# Reading file
with open(snip_spec) as f: data = f.read()

#-------------------------------------------------------------------------------
# Searching spec
try:
  _, spec, *_ = data.split(spec_beg_specifier)
  spec, _ = spec.split(spec_end_specifier)
except:
  print("No snip spec found")
  exit(1)

#-------------------------------------------------------------------------------
# Parsing spec into token matrix

# Split spec string into lines
lines = spec.split("\n")

# Split lines into tokens
token_matrix = [line.split("|") for line in lines]

# Strip whitespace from tokens
token_matrix = [[token.lstrip().rstrip() for token in token_line]
                for token_line in token_matrix]

# Remove lines with less than 3 tokens
token_matrix = [line for line in token_matrix if len(line) >= 3]

# Remove lines without a numeric second token (corresponding to snip_id)
token_matrix = [(snip_id, snip_start_time)
                for _, snip_id, snip_start_time, *_ in token_matrix
                if snip_id.isnumeric()]

# Parsing start times into seconds
def parse_start_time(time_str):
  FPS = 30
  h, m, s = time_str.split(":")
  h, m, s = int(h), int(m), int(s)
  time_in_secs = h*3600 + m*60 + s
  return time_in_secs

token_matrix = [(snip_id, parse_start_time(snip_start_time)) for snip_id, snip_start_time in token_matrix]

# Printing snip_id, snip_start_time pairs:
[print(f"{snip_id}:{snip_start_time}", end=" ")
 for snip_id, snip_start_time in token_matrix]
