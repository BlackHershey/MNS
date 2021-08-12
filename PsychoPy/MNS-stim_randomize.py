""" Randomize stim conditions and rhythm types for the MNS study """
import random
import csv

# MNS subject numbers, 101 through 142 (42 total subjects, divisible by 6 and 2)
MNS_subject_numbers = range(101,143,1)
N = len(MNS_subject_numbers)

# build stim conditions (1 through 6) and rhythm types (A or B)
MNS_stim_conditions = []
MNS_rhythm_types = []
for i in range(0,7):
    MNS_stim_conditions += [1,2,3,4,5,6]

for i in range(0,21):
    MNS_rhythm_types += ['A','B']

# choose explicit random seed, so we get the same shuffle/sampling results every time
random.seed(0)

# loop until MNS101 is 1 and A
while True:
    MNS_shuffled_stim_conditions = random.sample(MNS_stim_conditions, k=N)
    MNS_shuffled_rhythm_types = random.sample(MNS_rhythm_types, k=N)
    if ( MNS_shuffled_stim_conditions[0] == 1 ) and ( MNS_shuffled_rhythm_types[0] in 'A' ):
        print(MNS_shuffled_stim_conditions)
        print(MNS_shuffled_rhythm_types)
        with open('MNS-stim_session_settings.tsv', 'wt', newline='', encoding='utf-8') as out_file:
            tsv_writer = csv.writer(out_file, delimiter='\t')
            tsv_writer.writerow(['participant', 'rhythm_group', 'one_min_order'])
            for idx, subject_number in enumerate(MNS_subject_numbers):
                tsv_writer.writerow([subject_number, MNS_shuffled_rhythm_types[idx], MNS_shuffled_stim_conditions[idx]])
            tsv_writer.writerow([999, 'A', 999])
        break