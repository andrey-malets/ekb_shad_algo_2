template<typename Raw, typename Index, typename Compressed>
void compactify_to(const Raw& raw, Index n, Compressed* out) {
  Index src = 0;
  for (const auto i : raw) {
    for (; src != i.src; ++src)
      out->end_row();
    out->append(i.dst);
  }

  for (; src != n; ++src)
    out->end_row();
}
